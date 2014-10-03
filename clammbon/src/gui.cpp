#include <iostream>
#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "gui.hpp"
#include "data_type.hpp"
#include "splitter.hpp"
#include "impl/gui_move.ipp"
#include <sort_algorithm/compare.hpp>

namespace gui
{
    void destroy_all_window()
    {
        Fl::first_window()->hide(); // 違うけどまあいいか…．delete系にすると2重開放で怒られる
        return;
    }

    std::future<int> make_window(image_type const& image, std::string const& window_name)
    {
        splitter sp;

        auto const cloned_image = std::make_shared<unfold_image_type>(sp.unfold_image(image));
        auto const height = image.rows;
        auto const width  = image.cols;

        return std::async(
            std::launch::async,
            [=]() -> int
            {
                auto window = std::make_unique<Fl_Window>(width, height, window_name.c_str());
                auto box    = std::make_unique<Fl_Box>(0, 0, width, height);

                Fl_RGB_Image image(cloned_image->data(), width, height);
                box->image(image);

                window->show();
                return Fl::run();
            }
        );
    }
    
    std::future<std::vector<std::vector<point_type>>> make_mansort_window(
        split_image_type const& splitted,
        std::string const& window_name
        )
    {
        return std::async(
            std::launch::async,
            [=/*, splitted=std::move(splitted)*/]() -> std::vector<std::vector<point_type>>
            {
                impl::MoveWindow window(splitted, window_name);
                window.show();

                Fl::run();
                return window.result();
            }
        );
    }

	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<answer_type_y> const& answer)
	{
		int const one_picx = data_.size.first / data_.split_num.first;
		int const one_picy = data_.size.second / data_.split_num.second;

		splitter sp;//どこからか持ってきてたsplitter
		split_image_type splitted = sp.split_image(data_);
		for (int c = 0; c < answer.size(); ++c){
			cv::Mat comb_pic(cv::Size(one_picx*answer.at(c).points.at(0).size(), one_picy*answer.at(c).points.size()), CV_8UC3);
			for (int i = 0; i < data_.split_num.second; ++i){
				for (int j = 0; j < data_.split_num.first; ++j){
					if (answer.at(c).points[i][j].y >= data_.split_num.second || answer.at(c).points[i][j].y <0 || answer.at(c).points[i][j].x >= data_.split_num.first || answer.at(c).points[i][j].x < 0) continue;
					cv::Rect roi_rect(j*one_picx, i*one_picy, one_picx, one_picy);
					cv::Mat roi(comb_pic, roi_rect);
					splitted[answer[c].points[i][j].y][answer[c].points[i][j].x].copyTo(roi);
					roi_rect.x += (data_.size.first / data_.split_num.first);
				}
			}
			std::ostringstream outname;
			if (answer[c].score > c && answer[c].score != 0)outname << "score = " << answer[c].score;
			else outname << "score = " << form_evaluate(data_, comp_, answer[c].points);
			cv::namedWindow(outname.str(), CV_WINDOW_AUTOSIZE);
			cv::imshow(outname.str(), comb_pic);
		}
		cv::waitKey(0);
	}

	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<std::vector<point_type>>> const& matrix)
	{
		int const one_picx = data_.size.first / data_.split_num.first;
		int const one_picy = data_.size.second / data_.split_num.second;
		splitter sp;//どこからか持ってきてたsplitter
		split_image_type splitted = sp.split_image(data_);
		for (int c = 0; c < matrix.size(); ++c){
			if (matrix.at(c).size() == 1 && matrix.at(c).at(0).size() == 1)continue;
			cv::Mat comb_pic(cv::Size(one_picx*matrix.at(c).at(0).size(), one_picy*matrix.at(c).size()), CV_8UC3);
			for (int i = 0; i < matrix.at(c).size(); ++i){
				for (int j = 0; j < matrix.at(c).at(0).size(); ++j){
					if (matrix.at(c)[i][j].y >= data_.split_num.second || matrix.at(c)[i][j].y <0 || matrix.at(c)[i][j].x >= data_.split_num.first || matrix.at(c)[i][j].x < 0) continue;
					cv::Rect roi_rect(j*one_picx, i*one_picy, one_picx, one_picy);
					cv::Mat roi(comb_pic, roi_rect);
					splitted[matrix.at(c)[i][j].y][matrix.at(c)[i][j].x].copyTo(roi);
					roi_rect.x += (data_.size.first / data_.split_num.first);
				}
			}
			std::ostringstream outname;
			outname << "combine show image " << c;
			cv::namedWindow(outname.str(), CV_WINDOW_AUTOSIZE);
			cv::imshow(outname.str(), comb_pic);
		}
		cv::waitKey(0);
		for (int c = 0; c < matrix.size(); ++c)
		{
			const std::string outname = std::string("combine show image ") + std::to_string(c);
			std::cout << outname << std::endl;
			cv::destroyWindow(outname);
		}
	}

	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type>>const& matrix)
	{
		int const one_picx = data_.size.first / data_.split_num.first;
		int const one_picy = data_.size.second / data_.split_num.second;

		cv::Mat comb_pic(cv::Size(one_picx*matrix.at(0).size(), one_picy*matrix.size()), CV_8UC3);
		splitter sp;//どこからか持ってきてたsplitter
		split_image_type splitted = sp.split_image(data_);

		for (int i = 0; i < matrix.size(); ++i){
			for (int j = 0; j < matrix.at(0).size(); ++j){
				if (matrix[i][j].y >= data_.split_num.second || matrix[i][j].y <0 || matrix[i][j].x >= data_.split_num.first || matrix[i][j].x < 0) continue;
				cv::Rect roi_rect(j*one_picx, i*one_picy, one_picx, one_picy);
				cv::Mat roi_mat(comb_pic, roi_rect);
				splitted[matrix[i][j].y][matrix[i][j].x].copyTo(roi_mat);
			}
		}
		std::ostringstream outname("combine_show_image");
			cv::namedWindow(outname.str(), CV_WINDOW_AUTOSIZE);
			cv::imshow(outname.str(), comb_pic);

		cvWaitKey(1);
	}

	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type>>const& matrix, std::ostringstream const outname)
	{
		int const one_picx = data_.size.first / data_.split_num.first;
		int const one_picy = data_.size.second / data_.split_num.second;

		cv::Mat comb_pic(cv::Size(one_picx*matrix.at(0).size(), one_picy*matrix.size()), CV_8UC3);
		splitter sp;//どこからか持ってきてたsplitter
		split_image_type splitted = sp.split_image(data_);

		for (int i = 0; i < matrix.size(); ++i){
			for (int j = 0; j < matrix.at(0).size(); ++j){
				if (matrix[i][j].y >= data_.split_num.second || matrix[i][j].x >= data_.split_num.first) continue;
				cv::Rect roi_rect(j*one_picx, i*one_picy, one_picx, one_picy);
				cv::Mat roi_mat(comb_pic, roi_rect);
				splitted[matrix[i][j].y][matrix[i][j].x].copyTo(roi_mat);
			}
		}
		cv::namedWindow(outname.str(), CV_WINDOW_AUTOSIZE);
		cv::imshow(outname.str(), comb_pic);

		cvWaitKey(0);
	}

	void show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<answer_type_y> const& answer)
	{
		for (auto& one_answer : answer)
	{
			std::ostringstream outname;
			outname.str("");
			if (one_answer.score != 0)outname << "score = " << one_answer.score;
			else outname << "score = " << form_evaluate(data_, comp_, one_answer.points);
			cv::namedWindow(outname.str(), CV_WINDOW_AUTOSIZE);
			cv::imshow(outname.str(), one_answer.mat_image);
		}
		cv::waitKey(10000);
	}

} // namespace gui


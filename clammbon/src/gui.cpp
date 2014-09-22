#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "gui.hpp"
#include "data_type.hpp"
#include "splitter.hpp"
#include "impl/gui_move.ipp"
#include <sort_algorithm/compare.hpp>

namespace gui
{
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
        question_raw_data const& data,
        std::string const& window_name
        )
    {
        splitter sp;
        auto splitted = sp.split_image(data);

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

	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, answer_type_y const& answer)
	{
		cv::Mat comb_pic(cv::Size(data_.size.first, data_.size.second), CV_8UC3);
		cv::Rect roi_rect;
		roi_rect.height = (data_.size.second / data_.split_num.second);// picy/sepy
		roi_rect.width = (data_.size.first / data_.split_num.first);// picx/sepx
		splitter sp;//どこからか持ってきてたsplitter
		split_image_type splitted = sp.split_image(data_);
		for (int c = 0; c < answer.points.size(); ++c){
			for (int i = 0; i < data_.split_num.second; ++i){
				for (int j = 0; j < data_.split_num.first; ++j){
					cv::Mat roi(comb_pic, roi_rect);
					splitted[answer.points[c][i][j].y][answer.points[c][i][j].x].copyTo(roi);
					roi_rect.x += (data_.size.first / data_.split_num.first);
				}
				roi_rect.x = 0;
				roi_rect.y += (data_.size.second / data_.split_num.second);
			}
			std::ostringstream outname;
			outname.str("");
			if (answer.score.size() > c && answer.score[c] != 0)outname << "score = " << answer.score[c];
			else outname << "score = " << form_evaluate(comp_, answer.points[c]);
			cv::namedWindow(outname.str(), CV_WINDOW_AUTOSIZE);
			cv::imshow(outname.str(), comb_pic);
		}
		cvWaitKey(0);
	}

	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type>>const& matrix)
	{
		int const one_picx = data_.size.first / data_.split_num.first;
		int const one_picy = data_.size.second / data_.split_num.second;
		
		cv::Mat comb_pic(cv::Size(one_picx*matrix.at(0).size(),one_picy*matrix.size()), CV_8UC3);
		splitter sp;//どこからか持ってきてたsplitter
		split_image_type splitted = sp.split_image(data_);

		for (int i = 0; i < matrix.size(); ++i){
			for (int j = 0; j < matrix.at(0).size(); ++j){
				if (matrix[i][j].y > data_.split_num.second || matrix[i][j].x > data_.split_num.first) continue;
				cv::Rect roi_rect(j*one_picx, i*one_picy, one_picx, one_picy);
				cv::Mat roi_mat(comb_pic, roi_rect);
				splitted[matrix[i][j].y][matrix[i][j].x].copyTo(roi_mat);
			}
		}
		std::ostringstream outname("combine_show_image"); 
		cv::namedWindow(outname.str(), CV_WINDOW_AUTOSIZE);
		cv::imshow(outname.str(), comb_pic);

		cvWaitKey(0);
	}

	void show_image(question_raw_data const& data_, compared_type const& comp_, answer_type_y const& answer)
	{
		for (int i = 0; i < answer.points.size(); ++i){
			std::ostringstream outname;
			outname.str("");
			if (answer.score.size() > i && answer.score[i] != 0)outname << "score = " << answer.score[i];
			else outname << "score = " << form_evaluate(comp_, answer.points[i]);
			cv::namedWindow(outname.str(), CV_WINDOW_AUTOSIZE);
			cv::imshow(outname.str(), answer.mat_image[i]);
		}
		cvWaitKey(0);
	}

} // namespace gui


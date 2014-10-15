#include <iostream>
#include <memory>
#include <boost/make_shared.hpp>
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

#include <boost/thread.hpp>

namespace gui
{
    manager::manager(std::function<void(std::vector<std::vector<point_type>> const&)> const& callback)
    {
        th_ = std::make_unique<boost::thread>(
            [this, callback]()
            {
                // GUI Thread
                while(true)
                {
                    if(!queue_.empty())
                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        for(auto const& f : queue_) windows_.push_back(f());
                        queue_.clear();
                    }

                    for(auto it = windows_.begin(); it != windows_.end();)
                    {
                        auto res = gui::get_result(*it);
                        if (res) callback(res.get()); // 解答

                        if(gui::is_hide_window(*it)) it = windows_.erase(it);
                        else                         ++it;
                    }

                    Fl::wait(0);
                }
            });
    }

    manager::~manager()
    {
        th_->join();
        th_.reset();
    }

    void manager::wait_all_window()
    {
        while(!windows_.empty());
        return;
    }

    void manager::push_back(std::function<window_ptr()> const& func)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(func);
    }

    boost::shared_ptr<impl::MoveWindow> make_mansort_window(
        split_image_type const& splitted,
        std::vector<std::vector<point_type>> const& default_position,
        std::string const& window_name
        )
    {
        auto window = boost::make_shared<impl::MoveWindow>(splitted, default_position, window_name);
        window->show();

        return window;
    }
    
    boost::shared_ptr<impl::MoveWindow> make_mansort_window(
        split_image_type const& splitted,
        std::string const& window_name
        )
    {
        int const split_x = splitted.at(0).size();
        int const split_y = splitted.size();

        std::vector<std::vector<point_type>> position;
        position.reserve(split_y);
        for(int i=0; i<split_y; ++i)
        {
            std::vector<point_type> line;
            line.reserve(split_x);
            for(int j=0; j<split_x; ++j)
            {
                line.push_back(point_type{j, i});
            }
            position.push_back(std::move(line));
        }

        return make_mansort_window(splitted, position, window_name);
    }
    
    boost::optional<std::vector<std::vector<point_type>>> get_result(boost::shared_ptr<impl::MoveWindow>& ptr)
    {
        if(ptr->wait_submit()) return ptr->result();
        else                   return boost::none;
    }

    bool is_hide_window(boost::shared_ptr<impl::MoveWindow>& ptr)
    {
        return !ptr->visible();
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
					if (matrix.at(c)[i][j].y >= data_.split_num.second || matrix.at(c)[i][j].y < 0 || matrix.at(c)[i][j].x >= data_.split_num.first || matrix.at(c)[i][j].x < 0)
					{
						cv::rectangle(comb_pic, cv::Point(j*one_picx, i*one_picy), cv::Point((j + 1)*one_picx, (i + 1)*one_picy), cv::Scalar(128, 128, 128), 3, 4);
						continue;
					}
					cv::Rect roi_rect(j*one_picx, i*one_picy, one_picx, one_picy);
					cv::Mat roi(comb_pic, roi_rect);
					splitted[matrix.at(c)[i][j].y][matrix.at(c)[i][j].x].copyTo(roi);
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
				if (matrix[i][j].y >= data_.split_num.second || matrix[i][j].y < 0 || matrix[i][j].x >= data_.split_num.first || matrix[i][j].x < 0)
				{
					cv::rectangle(comb_pic, cv::Point(j*one_picx, i*one_picy), cv::Point((j + 1)*one_picx, (i + 1)*one_picy), cv::Scalar(128, 128, 128), 3, 4);
					continue;
				}
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

	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type>>const& matrix, std::ostringstream const outname)
	{
		int const one_picx = data_.size.first / data_.split_num.first;
		int const one_picy = data_.size.second / data_.split_num.second;

		cv::Mat comb_pic(cv::Size(one_picx*matrix.at(0).size(), one_picy*matrix.size()), CV_8UC3);
		splitter sp;//どこからか持ってきてたsplitter
		split_image_type splitted = sp.split_image(data_);

		for (int i = 0; i < matrix.size(); ++i){
			for (int j = 0; j < matrix.at(0).size(); ++j){
				if (matrix[i][j].y >= data_.split_num.second || matrix[i][j].y < 0 || matrix[i][j].x >= data_.split_num.first || matrix[i][j].x < 0)
				{
					cv::rectangle(comb_pic, cv::Point(j*one_picx, i*one_picy), cv::Point((j + 1)*one_picx, (i + 1)*one_picy), cv::Scalar(128, 128, 128), 3, 4);
					continue;
				}
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


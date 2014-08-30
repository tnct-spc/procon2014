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

	void show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<std::vector<point_type>>>const & matrix)
	{
		cv::Mat comb_pic(cv::Size(data_.size.first, data_.size.second), CV_8UC3);
		cv::Rect roi_rect;
		roi_rect.height = (data_.size.second / data_.split_num.second);// picy/sepy
		roi_rect.width = (data_.size.first / data_.split_num.first);// picx/sepx
		splitter sp;//どこからか持ってきてたsplitter
		split_image_type splitted = sp.split_image(data_);
		for (auto arr:matrix){
			for (int i = 0; i < data_.split_num.second; i++){
				for (int j = 0; j < data_.split_num.first; j++){
					cv::Mat roi(comb_pic, roi_rect);
					splitted[arr[i][j].y][arr[i][j].x].copyTo(roi);
					roi_rect.x += (data_.size.first / data_.split_num.first);
				}
				roi_rect.x = 0;
				roi_rect.y += (data_.size.second / data_.split_num.second);
			}
			std::ostringstream outname;
			outname.str("");
			outname << "num score="<< form_evaluate(comp_,arr);
			cv::namedWindow(outname.str(), CV_WINDOW_AUTOSIZE);
			cv::imshow(outname.str(), comb_pic);

		}
		cvWaitKey(0);
	}

} // namespace gui


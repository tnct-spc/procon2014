#ifndef CLAMMBON_GUI_HPP
#define CLAMMBON_GUI_HPP

#include <future>
#include "data_type.hpp"
#include "splitter.hpp"

namespace gui
{
    std::future<int> make_window(image_type const& image, std::string const& window_name = "Untitled");

    std::future<std::vector<std::vector<point_type>>> make_mansort_window(
        question_raw_data const& data,
        std::string const& window_name = ""
        );

	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<answer_type_y> const& answer);
	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type>>const& matrix);
	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type>>const& matrix,std::ostringstream const outname);

	void show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<answer_type_y> const& answer);

} // namespace gui

#endif


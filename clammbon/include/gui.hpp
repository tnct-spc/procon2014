#ifndef CLAMMBON_GUI_HPP
#define CLAMMBON_GUI_HPP

#include <future>
#include "data_type.hpp"
#include "splitter.hpp"

namespace gui
{
    namespace impl{ class MoveWindow; }

    int wait_all_window();
    
    boost::shared_ptr<impl::MoveWindow> make_mansort_window(
        split_image_type const& splitted,
        std::vector<std::vector<point_type>> const& default_position,
        std::string const& window_name = ""
        );

    boost::shared_ptr<impl::MoveWindow> make_mansort_window(
        split_image_type const& splitted,
        std::string const& window_name = ""
        );
    
    boost::optional<std::vector<std::vector<point_type>>> get_result(boost::shared_ptr<impl::MoveWindow>& ptr);

	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<answer_type_y> const& answer);
	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type>>const& matrix);
	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<std::vector<point_type>>> const& matrix);
	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type>>const& matrix,std::ostringstream const outname);

	void show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<answer_type_y> const& answer);

} // namespace gui

#endif


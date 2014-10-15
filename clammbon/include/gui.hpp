#ifndef CLAMMBON_GUI_HPP
#define CLAMMBON_GUI_HPP

#include <future>
#include <boost/thread.hpp>
#include "data_type.hpp"
#include "splitter.hpp"

namespace gui
{
    namespace impl{ class MoveWindow; }

    // GUI Thread係
    class manager : boost::noncopyable {
    public:
        typedef boost::shared_ptr<impl::MoveWindow> window_ptr;
        manager(std::function<void(std::vector<std::vector<point_type>> const&)> const& callback);
        virtual ~manager();

        void wait_all_window();
        void push_back(std::function<window_ptr()> const& func);

    private:
        std::mutex mutex_;
        std::unique_ptr<boost::thread> th_;

        std::vector<std::function<window_ptr()>> queue_;
        std::vector<window_ptr> windows_;
    };
        
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
    bool is_hide_window(boost::shared_ptr<impl::MoveWindow>& ptr);

	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<answer_type_y> const& answer);
	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type>>const& matrix);
	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<std::vector<point_type>>> const& matrix);
	void combine_show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type>>const& matrix,std::ostringstream const outname);

	void show_image(question_raw_data const& data_, compared_type const& comp_, std::vector<answer_type_y> const& answer);

} // namespace gui

#endif


#include <memory>
#include "gui.hpp"
#include "data_type.hpp"
#include "splitter.hpp"
#include "impl/gui_move.ipp"

namespace gui
{
    std::future<int> make_window(image_type const& image, std::string const& window_name)
    {
        splitter sp;

        auto const cloned_image = std::make_shared<unfold_image_type>(sp.unfold_image(image));
        auto const height = image.size();
        auto const width  = image.at(0).size();

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
} // namespace gui


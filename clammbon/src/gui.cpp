#include <memory>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include "gui.hpp"
#include "splitter.hpp"

namespace gui
{
    std::future<int> make_window(image_type const& image)
    {
        splitter sp;

        auto const cloned_image = std::make_shared<unfold_image_type>(sp.unfold_image(image));
        auto const height = image.size();
        auto const width  = image.at(0).size();

        return std::async(
            std::launch::async,
            [=]() -> int
            {
                auto window = std::make_unique<Fl_Window>(width, height, "Test Window");
                auto box    = std::make_unique<Fl_Box>(0, 0, width, height);

                Fl_RGB_Image image(cloned_image->data(), width, height);
                box->image(image);

                window->show();
                return Fl::run();
            }
        );
    }
} // namespace gui


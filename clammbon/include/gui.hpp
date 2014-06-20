#ifndef CLAMMBON_GUI_HPP
#define CLAMMBON_GUI_HPP

#include <future>
#include "data_type.hpp"
#include "splitter.hpp"

namespace gui
{
    std::future<int> make_window(image_type const& image, std::string const& window_name = "Untitled");

    class MoveBox : public Fl_Box
    {
    public:
        MoveBox(int const x, int const y, int const w, int const h);
        virtual ~MoveBox() = default;

    private:
        void position_swap(MoveBox* lhs, MoveBox* rhs);
        int handle(int const event) override;
    };
    
    class MoveWindow : public Fl_Window
    {
    public:
        MoveWindow(split_image_type const& images, std::string const& window_name);
        virtual ~MoveWindow() = default;
    
    private:
        splitter sp_;
        std::string const window_name_;
        std::vector<std::vector<unfold_image_type>> image_;
        std::vector<std::vector<Fl_RGB_Image*>> rgbs_;
        std::vector<std::vector<std::unique_ptr<MoveBox>>> boxes_;
    };

} // namespace gui

#endif


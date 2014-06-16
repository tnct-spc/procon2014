#ifndef CLAMMBON_GUI_HPP
#define CLAMMBON_GUI_HPP

#include <future>
#include "data_type.hpp"

namespace gui
{
    std::future<int> make_window(image_type const& image);
} // namespace gui

#endif


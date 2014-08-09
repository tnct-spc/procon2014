﻿#ifndef CLAMMBON_GUI_HPP
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

} // namespace gui

#endif


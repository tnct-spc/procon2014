#include <limits>
#include "splitter.hpp"

split_image_type splitter::split_image(question_raw_data const& raw) const
{
    int const parts_width = raw.size.first / raw.split_num.first;
    int const parts_height = raw.size.second / raw.split_num.second;

    split_image_type split_pixels;
    split_pixels.reserve(raw.split_num.second);
    
    for(int i=0; i<raw.split_num.second; ++i)
    {
        std::vector<image_type> line;
        line.reserve(raw.split_num.first);

        for(int j=0; j<raw.split_num.first; ++j)
        {
            cv::Rect roi(
                parts_width*j, parts_height*i,
                parts_width  , parts_height
                );

            line.push_back(raw.pixels(roi).clone());
        }

        split_pixels.push_back(std::move(line));
    }

    return split_pixels;
}

image_type splitter::join_image(split_image_type const& source) const
{
    int const part_width   = source[0][0].cols;
    int const part_height  = source[0][0].rows;
    int const split_width  = source[0].size();
    int const split_height = source.size();
    int const join_width   = part_width  * split_width;
    int const join_height  = part_height * split_height;

    image_type joined(join_height, join_width, CV_8UC3);
    for(int i=0; i<split_height; ++i)
    {
        for(int j=0; j<split_width; ++j)
        {
            cv::Rect roi(
                part_width*j, part_height*i,
                part_width  , part_height
                );

            source[i][j].copyTo(joined(roi));
        }
    }

    return joined;
}

unfold_image_type splitter::unfold_image(image_type const& fold_image) const
{
    int const width  = fold_image.cols;
    int const height = fold_image.rows;

    unfold_image_type unfold_image;
    unfold_image.reserve(3 * height * width);

    for(auto const& pixel : cv::Mat_<cv::Vec3b>(fold_image))
    {
        unfold_image.push_back(pixel[2]); // R
        unfold_image.push_back(pixel[1]); // G
        unfold_image.push_back(pixel[0]); // B
    }
    
    return unfold_image;
}

image_type splitter::fold_image(unfold_image_type const& unfold_image, int const height, int const width) const
{
    image_type fold_image(height, width, CV_8UC3);

    auto it = unfold_image.begin();
    for(auto& pixel : cv::Mat_<cv::Vec3b>(fold_image))
    {
        pixel[2] = *it++; // R
        pixel[1] = *it++; // G
        pixel[0] = *it++; // B
    }

    assert(it == unfold_image.end());

    return fold_image;
}


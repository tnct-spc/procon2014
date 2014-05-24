#include <limits>
#include "splitter.hpp"

split_image_type splitter::split_image(question_raw_data const& raw) const
{
    int const parts_width = raw.size.first / raw.split_num.first;
    int const parts_height = raw.size.second / raw.split_num.second;

    std::vector<std::vector<std::vector<std::vector<pixel_type>>>> split_pixels(
        raw.split_num.second,
        std::vector<std::vector<std::vector<pixel_type>>>(
            raw.split_num.first,
            std::vector<std::vector<pixel_type>>(
                parts_height,
                std::vector<pixel_type>(
                    parts_width,
                    {0,0,0}
                )
            )
        )
    );

    for(int i=0; i<raw.split_num.second; ++i) for(int j=0; j<raw.split_num.first; ++j)
    {
        int const splitting_pos = i * raw.split_num.first + j;
        for(int k=0; k<parts_height; ++k) for(int l=0; l<parts_width; ++l)
        {
            split_pixels[i][j][k][l] = raw.pixels[parts_height * i + k][parts_width * j + l];
        }
    }

    return split_pixels;
}


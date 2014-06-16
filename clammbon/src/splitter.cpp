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

image_type splitter::join_image(split_image_type const& source) const
{
    int const part_width   = source[0][0][0].size();
    int const part_height  = source[0][0].size();
    int const split_width  = source[0].size();
    int const split_height = source.size();
    int const join_width   = part_width  * split_width;
    int const join_height  = part_height * split_height;

    image_type joined(
        join_height,
        std::vector<pixel_type>(join_width, pixel_type{0,0,0})
        );

    for(int i=0; i<join_height; ++i)
    {
        for(int j=0; j<join_width; ++j)
        {
            joined[i][j] = source[i/part_height][j/part_width][i%part_height][j%part_width];
        }
    }

    return joined;
}

unfold_image_type splitter::unfold_image(image_type const& fold_image) const
{
    int const height = fold_image.size();
    int const width  = fold_image[0].size();

    unfold_image_type unfold_image(3 * height * width, 0);
    for(int i=0; i<height; ++i)
    {
        for(int j=0; j<width; ++j)
        {
            unfold_image[3*(i*width+j)+0] = fold_image[i][j].r;
            unfold_image[3*(i*width+j)+1] = fold_image[i][j].g;
            unfold_image[3*(i*width+j)+2] = fold_image[i][j].b;
        }
    }

    return unfold_image;
}

image_type splitter::fold_image(unfold_image_type const& unfold_image, int const height, int const width) const
{
    image_type fold_image(
        height,
        std::vector<pixel_type>(width, pixel_type{0,0,0})
        );

    for(int i=0; i<height; ++i)
    {
        for(int j=0; j<width; ++j)
        {
            fold_image[i][j].r = unfold_image[3*(i*width+j)+0];
            fold_image[i][j].g = unfold_image[3*(i*width+j)+1];
            fold_image[i][j].b = unfold_image[3*(i*width+j)+2];
        }
    }

    return fold_image;
}


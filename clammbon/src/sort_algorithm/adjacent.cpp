#include <limits>
#include <vector>
#include <data_type.hpp>
#include <sort_algorithm/adjacent.hpp>

auto select_minimum(compared_type const& compared_data) -> adjacent_type
{
    auto const height = compared_data.size();
    auto const width = compared_data.at(0).size();

    adjacent_type adjacent_data(
        height,
        std::vector<direction_type<point_type>>(width)
        );

    for(int i=0; i<height; ++i) for(int j=0; j<width; ++j)
    {
        uint64_t norm_rl, norm_lr, norm_ud, norm_du;
        norm_rl = norm_lr = norm_ud = norm_du = std::numeric_limits<uint64_t>::max();

        for(int k=0; k<height; ++k) for(int l=0; l<width; ++l)
        {
            if(i==k && j==l) continue;

            if(compared_data[i][j][k][l].right < norm_rl)
            {
                norm_rl = compared_data[i][j][k][l].right;
                adjacent_data[i][j].right = point_type{l, k};
            }
            if(compared_data[i][j][k][l].left < norm_lr)
            {
                norm_lr = compared_data[i][j][k][l].left;
                adjacent_data[i][j].left = point_type{l, k};
            }
            if(compared_data[i][j][k][l].up < norm_ud)
            {
                norm_ud = compared_data[i][j][k][l].up;
                adjacent_data[i][j].up = point_type{l, k};
            }
            if(compared_data[i][j][k][l].down < norm_du)
            {
                norm_du = compared_data[i][j][k][l].down;
                adjacent_data[i][j].down = point_type{l, k};
            }
        }
    }

    return adjacent_data;
}

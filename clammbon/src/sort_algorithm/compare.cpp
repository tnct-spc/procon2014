#include <limits>
#include <vector>
#include <data_type.hpp>
#include <sort_algorithm/compare.hpp>

point_type ur_choose(compared_type const& comp, point_type const ul, point_type const dl, point_type const dr)
{
    point_type possition;
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[dr.y][dr.x][i][j].up + comp[ul.y][ul.x][i][j].right;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

point_type ul_choose(compared_type const& comp, point_type const ur, point_type const dl, point_type const dr)
{
    point_type possition;
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[dl.y][dl.x][i][j].up + comp[ur.y][ur.x][i][j].left;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

point_type dr_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dl)
{
    point_type possition;
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[ur.y][ur.x][i][j].down + comp[dl.y][dl.x][i][j].right;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

point_type dl_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dr)
{
    point_type possition;
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[ul.y][ul.x][i][j].down + comp[dr.y][dr.x][i][j].left;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

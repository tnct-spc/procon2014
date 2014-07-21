#ifndef CLAMMBON_SORT_COMPARE_HPP
#define CLAMMBON_SORT_COMPARE_HPP

#include <boost/range/algorithm.hpp>
#include <data_type.hpp>

point_type ur_choose(compared_type const& comp, point_type const ul, point_type const dl, point_type const dr);
point_type ul_choose(compared_type const& comp, point_type const ur, point_type const dl, point_type const dr);
point_type dr_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dl);
point_type dl_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dr);

#endif

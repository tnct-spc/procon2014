#ifndef CLAMMBON_SORT_COMPARE_HPP
#define CLAMMBON_SORT_COMPARE_HPP

#include <data_type.hpp>

point_type ur_choose(compared_type const& comp, point_type const ul, point_type const dl, point_type const dr);
point_type ul_choose(compared_type const& comp, point_type const ur, point_type const dl, point_type const dr);
point_type dr_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dl);
point_type dl_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dr);
uint_fast64_t form_evaluate(compared_type const& comp_,std::vector<std::vector<point_type> > const& matrix);
size_t get_kind_num(question_raw_data const& data_, std::vector<std::vector<point_type>> const& matrix, int const x, int const y);

#endif

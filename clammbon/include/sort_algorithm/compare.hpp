#ifndef CLAMMBON_SORT_COMPARE_HPP
#define CLAMMBON_SORT_COMPARE_HPP

#include <data_type.hpp>

point_type ur_choose(compared_type const& comp, point_type const ul, point_type const dl, point_type const dr);
point_type ul_choose(compared_type const& comp, point_type const ur, point_type const dl, point_type const dr);
point_type dr_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dl);
point_type dl_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dr);
point_type ur_choose(compared_type const& comp, point_type const ul, point_type const dl, point_type const dr, std::vector<point_type>& usable);
point_type ul_choose(compared_type const& comp, point_type const ur, point_type const dl, point_type const dr, std::vector<point_type>& usable);
point_type dr_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dl, std::vector<point_type>& usable);
point_type dl_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dr, std::vector<point_type>& usable);
point_type u_choose(compared_type const& comp, point_type const next_to, std::vector<point_type>& usable);
point_type r_choose(compared_type const& comp, point_type const next_to, std::vector<point_type>& usable);
point_type d_choose(compared_type const& comp, point_type const next_to, std::vector<point_type>& usable);
point_type l_choose(compared_type const& comp, point_type const next_to, std::vector<point_type>& usable);
uint_fast64_t form_evaluate(compared_type const& comp_,std::vector<std::vector<point_type> > const& matrix);
int get_kind_num(question_raw_data const& data_,std::vector<std::vector<point_type>> const& matrix, int const x, int const y);
uint_fast64_t range_evaluate(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type> > matrix, int x, int y);
std::vector<point_score> make_matrix_rgb_database(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type> >& matrix);
int get_matrix_average(std::vector<point_score>const& matrix);
int one_side_val(compared_type const& comp_, std::vector<std::vector<point_type> >matrix, point_type point);
std::vector<point_type> duplicate_delete(compared_type const& comp_, std::vector<std::vector<point_type> >& matrix);
int try_matrix_copy(std::vector<std::vector<point_type>>& sorted_matrix, std::vector<std::vector<point_type>>const& matrix, point_type point);
#endif

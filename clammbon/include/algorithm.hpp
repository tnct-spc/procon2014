#ifndef RESOLVER_ALGORITHM_HPP
#define RESOLVER_ALGORITHM_HPP

#include "data_type.hpp"

class algorithm
{
public:
    algorithm() = default;
    virtual ~algorithm() = default;

    void operator() (question_data const& data);

    void solve();
    void greedy();
    void brute_force();
    inline void print() const;
    const point_type current_point(point_type const& point) const;
    inline const bool is_sorted(point_type const& point) const;
    void sequential_move(point_type const& target, std::vector<HVDirection> const& directions);
    void move_selecting(HVDirection const& direction);
    void move_target(point_type const& target, HVDirection const& direction);
    void move_to(point_type const& to);

private:
    std::vector<std::vector<point_type>> matrix;
    std::vector<point_type> sorted_points;
    answer_type answer;
    point_type selecting;
    int width;
    int height;
    int cost_select;
    int cost_change;
    int score_select;
    int score_change;
    int sorted_row;
    int sorted_col;
};

#endif

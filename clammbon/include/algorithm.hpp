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
    std::string const& answer_tostr() const;
    inline void print() const;
    const point_type current_point(point_type const& point) const;
    Direction inverse_direction(Direction const& direction) const;
    void move_direction(point_type const& target, Direction const& direction);
    void move_from_to(point_type const& from, point_type const& to);
    void move_target_direction(point_type const& target, Direction const& direction, TurnSide const& turnside);

private:
    std::vector<std::vector<point_type>> matrix;
    std::vector<point_type> sorted_points;
    answer_list_t answer_list;
    point_type mover;
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

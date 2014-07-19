#ifndef RESOLVER_ALGORITHM_HPP
#define RESOLVER_ALGORITHM_HPP

#include "data_type.hpp"

class algorithm
{
public:
    explicit algorithm() = default;
    virtual ~algorithm() = default;

    void operator() (question_data const& data);

    void print();
    uint16_t point_to_num(point_type const& point);
    void solve();
    void greedy();
    void brute_force();
    point_type current_point(point_type const& point);
    Direction inverse_direction(Direction const& direction);
    void move_direction(point_type const& target, Direction const& direction);
    void move_from_to(point_type const& from, point_type const& to);
    void move_target_direction(point_type const& target, Direction const& direction, TurnSide const& turnside);

private:
    std::vector<std::vector<point_type>> matrix;
    std::vector<point_type> sorted_points;
    int width;
    int height;
    int sorted_row;
    int sorted_col;
    point_type mover;
};

#endif

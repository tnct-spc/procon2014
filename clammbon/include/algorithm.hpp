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
    int point_to_num(point_type const& point);
    void solve();
    void greedy();
    void brute_force();
    point_type current_point(point_type const& point);
    void move_direction(point_type const& target, int const& direction);
    void move_from_to(point_type from, point_type const& to);

private:
    std::vector<std::vector<point_type>> matrix;
    direction_type<int> directions;
    int width;
    int height;
    int sorted_row;
    int sorted_col;
    point_type mover_org;
};

#endif

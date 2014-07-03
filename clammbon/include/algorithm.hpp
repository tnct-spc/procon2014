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
    void move_from_to(point_type from, point_type const& to);
    void move_target_direction(point_type const& target, Direction const& direction, TurnSide const& turnside);

private:
    std::vector<std::vector<point_type>> matrix;
    int width;
    int height;
    int sorted_row;
    int sorted_col;
    point_type mover;
};

Direction inverse_direction(Direction const& direction) {
    // 逆向きの方向を返す
    if (direction == Direction::Up) {
        return Direction::Down;
    } else if (direction == Direction::Down) {
        return Direction::Up;
    } else if (direction == Direction::Right) {
        return Direction::Left;
    } else if (direction == Direction::Left) {
        return Direction::Right;
    }
}

#endif

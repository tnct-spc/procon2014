#ifndef RESOLVER_ALGORITHM_HPP
#define RESOLVER_ALGORITHM_HPP

#include "data_type.hpp"

class algorithm
{
public:
    explicit algorithm() = default;
    virtual ~algorithm() = default;

    void operator() (question_data const& data);

    int manhattan_distance(point_type& p0, point_type& p1);
    int distance_to_origial(point_type& point);
    bool check_finish();

private:
    std::vector<std::vector<point_type>> matrix;
    direction_type<int>* directions;
};

#endif

#ifndef RESOLVER_ALGORITHM_HPP
#define RESOLVER_ALGORITHM_HPP

#include "data_type.hpp"

class algorithm
{
public:
    explicit algorithm() = default;
    virtual ~algorithm() = default;

    void operator() (question_data const& data);
};

#endif

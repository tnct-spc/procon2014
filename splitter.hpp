#include "data_type.hpp"

class splitter
{
public:
    explicit splitter() = default;
    virtual ~splitter() = default;

    question_data operator() (question_raw_data const& raw);
};

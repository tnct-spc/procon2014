#ifndef RESOLVER_ALGORITHM_HPP
#define RESOLVER_ALGORITHM_HPP

#include <boost/optional.hpp>
#include "data_type.hpp"

class algorithm
{
public:
    typedef answer_type return_type; // TODO: あとで相談とか

    algorithm();
    virtual ~algorithm();

    auto get() -> boost::optional<return_type>;
    void reset(question_data const& data);

private:
    class impl;
    impl *pimpl_;
};

#endif

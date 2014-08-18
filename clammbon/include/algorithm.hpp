#ifndef RESOLVER_ALGORITHM_HPP
#define RESOLVER_ALGORITHM_HPP

#include <boost/optional.hpp>
#include <boost/coroutine/coroutine.hpp>
#include "data_type.hpp"

class algorithm
{
public:
    typedef answer_list return_type; // TODO: あとで相談とか

    algorithm() = default;
    virtual ~algorithm() = default;

    auto get() -> boost::optional<return_type>;
    void reset(question_data const& data);

private:
    void process(boost::coroutines::coroutine<return_type>::push_type& yield);
    void operator() (boost::coroutines::coroutine<return_type>::push_type& yield);

    boost::optional<question_data> data_;
    boost::coroutines::coroutine<return_type>::pull_type co_;
};

#endif

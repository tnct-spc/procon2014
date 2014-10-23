#ifndef RESOLVER_TRUNCATER_HPP
#define RESOLVER_TRUNCATER_HPP

#include <boost/optional.hpp>
#include "data_type.hpp"
#include "slide_algorithm/algorithm.hpp"

class truncater
{
public:
    typedef answer_type return_type;

    truncater();
    virtual ~truncater();

    auto get() -> boost::optional<return_type>;
    void reset(question_data const& data);

private:
    class impl;
    impl *pimpl_;
};

#endif

#ifndef SERVER_PROBLEM_HPP
#define SERVER_PROBLEM_HPP

#include "server.hpp"
#include <boost/lexical_cast.hpp>

class Problem
{
private:
    std::ostringstream outerr;
    std::string const problem_set;
    question_data question;
    bool sane;
public:
    Problem(std::string const problem_set) : problem_set(problem_set)
    {
        sane = false;
    }
    Problem() : problem_set("default")
    {
        sane = false;
    }
    void load(std::string const& problemid, std::string const& playerid = "guest");
    inline std::string const get_error()
    {
        return outerr.str();
    }
    inline question_data const& get()
    {
        return question;
    }
    inline bool valid()
    {
        return sane;
    }
};

#endif


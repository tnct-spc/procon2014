#ifndef SERVER_PROBLEM_HPP
#define SERVER_PROBLEM_HPP

#include "server.hpp"

class problem
{
private:
    std::ostringstream outerr;
    std::string const problem_set;
    question_data question;
public:
    problem(std::string const problem_set) : problem_set(problem_set)
    {}
    problem() : problem_set("default")
    {}
    void load(std::string const& problemid, std::string const& playerid = "guest");
    inline std::string const get_error()
    {
        return outerr.str();
    }
    inline question_data const& get()
    {
        return question;
    }
};

#endif


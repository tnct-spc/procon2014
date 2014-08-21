#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include "data_type.hpp"
#include "tools.hpp"

class pcserver
{
private:
    std::string problem_set;
public:
    pcserver()
    {
        problem_set = "default";
    }
    pcserver(std::string s)
    {
        problem_set = s;
    }
    std::ostringstream outerr, output;
    inline std::string const get_problem_set()
    {
        return problem_set;
    }
    void parse(question_data const& problem, answer_list const& answer);
    answer_list const string_to_answer(std::string const s);
};

#endif


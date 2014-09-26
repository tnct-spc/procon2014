#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include "data_type.hpp"
#include "tools.hpp"

class pcserver
{
private:
    std::string problem_set;
    std::ostringstream outerr, output;
    bool sane;
public:
    pcserver()
    {
        problem_set = "default";
        sane = false;
    }
    pcserver(std::string s)
    {
        problem_set = s;
        sane = false;
    }
    inline std::string const get_problem_set()
    {
        return problem_set;
    }
    void parse(question_data const& problem, answer_list const& answer);
    answer_list const string_to_answer(std::string const s);
    inline bool ok()
    {
        return sane;
    }
    inline std::string && get_error()
    {
        return std::move(outerr.str());
    }
    inline std::string && get_output()
    {
        return std::move(output.str());
    }
};

#endif


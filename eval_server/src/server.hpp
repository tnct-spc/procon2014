#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include "data_type.hpp"

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
    answer_list const string_to_answer(std::string const s);
    inline point_type const pos2int(std::string const pos)
    {
        point_type p;
        p.x = ('0' <= pos[0] && pos[0] <= '9') ? (pos[0] - '0') : (('A' <= pos[0] && pos[0] <= 'F') ? (pos[0] - 'A' + 10) : -1);
        p.y = ('0' <= pos[1] && pos[1] <= '9') ? (pos[1] - '0') : (('A' <= pos[1] && pos[1] <= 'F') ? (pos[1] - 'A' + 10) : -1);
        return p;
    }
    inline std::map<std::string, std::string> const post_req_to_map(std::string req)
    {
        std::map<std::string, std::string> m;
        std::vector<std::string> sep;
        boost::algorithm::split(sep, req, boost::is_any_of("&"));
        for(int i = 0; i < sep.size(); i++) {
            std::vector<std::string> tmp;
            boost::algorithm::split(tmp, sep[i], boost::is_any_of("="));
            m[sep[0]] = sep[1];
        }
        return m;
    }
    question_data const load_problem(std::string const& problemid, std::string const& playerid = "guest");
    void parse(question_data const& problem, answer_list const& answer);
    
};

#endif


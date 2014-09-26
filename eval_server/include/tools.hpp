#ifndef SERVER_TOOLS_HPP
#define SERVER_TOOLS_HPP

#include "server.hpp"
#include "config.hpp"

point_type const pos2int(std::string const pos);
std::map<std::string, std::string> const post_req_to_map(std::string req);
void percent_decode(std::string &s);
inline std::string const path_to_ppm(std::string const problem_set, std::string const problem_id)
{
    return PCS_PROBDIR + "/" + problem_set + "/problem/prob" + problem_id + ".ppm";
}
inline std::string const path_to_pos(std::string const problem_set, std::string const problem_id)
{
    return PCS_PROBDIR + problem_set + "/position/prob" + problem_id + ".pos";
}

#endif


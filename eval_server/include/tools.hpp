#ifndef SERVER_TOOLS_HPP
#define SERVER_TOOLS_HPP

#include "server.hpp"
#include "config.hpp"

std::vector<std::vector<point_type>> load_answer(std::string const& filename);
inline std::string const path_to_ppm(std::string const problem_set, std::string const problem_id)
{
    return PCS_PROBDIR + "/" + problem_set + "/problem/prob" + problem_id + ".ppm";
}
inline std::string const path_to_pos(std::string const problem_set, std::string const problem_id)
{
    return PCS_PROBDIR + "/" + problem_set + "/position/prob" + problem_id + ".pos";
}
inline std::string const path_to_ans(std::string const problem_set, std::string const problem_id)
{
    return PCS_PROBDIR + "/" + problem_set + "/answer/prob" + problem_id + ".ans";
}

#endif


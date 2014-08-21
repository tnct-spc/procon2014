#include "tools.hpp"

point_type const pos2int(std::string const pos)
{
    point_type p;
    p.x = ('0' <= pos[0] && pos[0] <= '9') ? (pos[0] - '0') : (('A' <= pos[0] && pos[0] <= 'F') ? (pos[0] - 'A' + 10) : -1);
    p.y = ('0' <= pos[1] && pos[1] <= '9') ? (pos[1] - '0') : (('A' <= pos[1] && pos[1] <= 'F') ? (pos[1] - 'A' + 10) : -1);
    return p;
}
std::map<std::string, std::string> const post_req_to_map(std::string req)
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


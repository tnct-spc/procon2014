#include "tools.hpp"
#include "data_type.hpp"

#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <sstream>
#include <string>

point_type const pos2int(std::string const pos)
{
    point_type p;
    p.x = ('0' <= pos[0] && pos[0] <= '9') ? (pos[0] - '0') : (('A' <= pos[0] && pos[0] <= 'F') ? (pos[0] - 'A' + 10) : -1);
    p.y = ('0' <= pos[1] && pos[1] <= '9') ? (pos[1] - '0') : (('A' <= pos[1] && pos[1] <= 'F') ? (pos[1] - 'A' + 10) : -1);
    return p;
}

void percent_decode(std::string &s)
{
    while(true) {
        std::string::size_type p = s.find("%");
        if(p == std::string::npos)
            break;
        char t[2];
        try {
            t[0] = std::stoi(s.substr(p + 1, 2), nullptr, 16);
        } catch(std::invalid_argument e) {
            std::cerr << "oops!\n";
            t[0] = '?';
        } catch(std::out_of_range e) {
            std::cerr << "hmm...\n";
            t[0] = '$';
        }
        t[1] = '\0';
        s.replace(p, 3, t);
    }
}

std::map<std::string, std::string> const post_req_to_map(std::string req)
{
    std::map<std::string, std::string> m;
    std::vector<std::string> sep;
    boost::algorithm::split(sep, req, boost::is_any_of("&"));
    for(size_t i = 0; i < sep.size(); i++) {
        std::vector<std::string> tmp;
        boost::algorithm::split(tmp, sep[i], boost::is_any_of("="));
        m[tmp[0]] = tmp[1];
    }
    return m;
}

// *.ans をロードするやつ
std::vector<std::vector<point_type>> load_answer(std::string const& filename)
{
    std::vector<std::vector<point_type>> r;
    std::ifstream ifs(filename);
    if(ifs.fail()) {
        std::cerr << "cannot load file " << filename << std::endl;
        return std::move(r);
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    std::string const src = oss.str();
    std::vector<std::string> lines;
    boost::algorithm::split(lines, src, boost::is_any_of("\r\n"));
//    std::cerr << lines.size() << " lines\n";
    for(int i = 0; i < lines.size(); i++) {
        if(lines[i] != "") {
            std::vector<std::string> points;
            boost::algorithm::split(points, lines[i], boost::is_any_of(" "));
            // points[j]の例: "(0,0)"
//            std::cerr << points.size() << " points\n";
            for(int j = 0; j < points.size(); j++) {
                if(points[j] != "") {
//                    std::cerr << "(" << i << "," << j << "): ";
                    std::vector<std::string> v;
                    boost::algorithm::split(v, points[j], boost::is_any_of(","));
                    int x = std::stoi(v[0].substr(1));
                    int y = std::stoi(v[1].substr(0, v[1].size() - 1));
//                    std::cerr << "(" << x << "," << y << ")\n";
                    // r.size() == rows
                    // r[foo].size() == cols
                    if(r.size() < y + 1)
                        r.resize(y + 1);
                    if(r[y].size() < x + 1)
                        r[y].resize(x + 1);
                    r[y][x].x = j;
                    r[y][x].y = i;
                }
            }
        }
    }
    std::cerr << "answer loaded\n";
    return std::move(r);
}


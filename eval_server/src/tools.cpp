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
    lines.erase(std::remove(lines.begin(), lines.end(), ""));
//    std::cerr << lines.size() << " lines\n";
    for(std::vector<std::string>::size_type i = 0; i < lines.size(); i++) {
        std::vector<std::string> points;
        boost::algorithm::split(points, lines[i], boost::is_any_of(" "));
        points.erase(std::remove(points.begin(), points.end(), ""));
        // points[j]の例: "(0,0)"
//        std::cerr << points.size() << " points\n";
        for(std::vector<std::string>::size_type j = 0; j < points.size(); j++) {
//            std::cerr << "(" << i << "," << j << "): ";
            std::vector<std::string> v;
            boost::algorithm::split(v, points[j], boost::is_any_of(","));
            int x = std::stoi(v[0].substr(1));
            int y = std::stoi(v[1].substr(0, v[1].size() - 1));
//            std::cerr << "(" << x << "," << y << ")\n";
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
    std::cerr << "answer loaded\n";
    return std::move(r);
}


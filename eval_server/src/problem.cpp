#include "problem.hpp"

//  🍣🍣🍣  問題ローダ 🍣🍣🍣
void problem::load(std::string const& problemid, std::string const& playerid)
{
    std::istringstream pos(path_to_pos(problem_set, playerid)), ppm(path_to_ppm(problem_set, playerid));
    std::string ppm_header, hash;
    std::pair<int, int> size;
    int problem_id, selectable, cost_select, cost_change;
    
    problem_id = std::stoi(problemid);

    ppm >> ppm_header;
    if(ppm_header != "P6")
        outerr << "ERROR: file format not recognized\n";
    ppm >> hash >> size.first >> size.second;
    ppm >> hash >> selectable;
    ppm >> hash >> cost_select >> cost_change;

    std::vector<std::vector<point_type>> block;
    for(int i = 0; i < size.second; i++) {
        std::vector<point_type> row;
        for(int j = 0; j < size.first; j++) {
            std::string s;
            pos >> s;
            row.push_back(pos2int(s));
        }
        block.push_back(row);
        row.clear(); // いるのか？
    }

    question = {
        problem_id,
        playerid,
        size,
        selectable,
        cost_select,
        cost_change,
        block,
    };
}


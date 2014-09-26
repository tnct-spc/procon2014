#include "problem.hpp"

//  🍣🍣🍣  問題ローダ 🍣🍣🍣
void Problem::load(std::string const& problemid, std::string const& playerid)
{
    std::ifstream pos(path_to_pos(problem_set, problemid)), ppm(path_to_ppm(problem_set, problemid));
    std::string ppm_header, hash;
    std::pair<int, int> size;
    int problem_id, selectable, cost_select, cost_change;
    
    try {
        problem_id = std::stoi(problemid);
    } catch (std::invalid_argument e) {
        outerr << "ERROR: invalid number: " + problemid << std::endl;
        return;
    } catch (std::out_of_range e) {
        outerr << "ERROR: out of range: " + problemid << std::endl;
        return;
    }

    if(pos.fail()) {
        outerr << "ERROR: failed to load position file " << path_to_pos(problem_set, problemid) << std::endl;
        return;
    }
    if(ppm.fail()) {
        outerr << "ERROR: failed to load problem file " << path_to_ppm(problem_set, problemid) << std::endl;
        return;
    }

    ppm >> ppm_header;
    if(ppm_header != "P6") {
        outerr << "ERROR: file format not recognized: \"" << ppm_header << "\"" << std::endl;
        return;
    }
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

    outerr << "STATUS: problem loaded successfully (problem id: " << problemid << ")" << std::endl;
    sane = true;
}


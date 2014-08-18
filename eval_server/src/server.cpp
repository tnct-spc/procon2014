#include "server.hpp"

// 🍣🍣🍣  解答変換  🍣🍣🍣
answer_list const pcserver::string_to_answer(std::string const s)
{
    answer_list al;
    answer_type a;
    std::istringstream ss;
    ss.str(s);

    int nl; // 選択回数
    ss >> nl;
    for(int i = 0; i < nl; i++) {
        std::string pos;
        ss >> pos;
        point_type ipos = pos2int(pos);
        if(ipos.x == -1 || ipos.y == -1) {
            outerr << "ERROR: illegal position: \"" + pos + "\"\n";
        } else {
            a.type = answer_type::action_type::select;
            a.position = ipos;
            a.direction = '\0';
            al.push_back(a);
        }
        int nx; // 交換回数
        ss >> nx;
        std::string move;
        ss >> move;
        for(int j = 0; j < nx; j++) {
            a.type = answer_type::action_type::change;
            a.position = ipos;
            a.direction = move[j];
            ipos = ipos.move_to(move[j]);
            if(ipos.x == -1 || ipos.y == -1) {
                outerr << "ERROR: illegal move: \'" << move[j] << "\'\n";
            } else {
                al.push_back(a);
            }
        }
    }
    return al;
}
//  🍣🍣🍣  問題ローダ 🍣🍣🍣
question_data const pcserver::load_problem(std::string const& problemid, std::string const& playerid)
{
    std::istringstream pos("../eval_server/position/prob" + problemid + ".pos"), ppm("../eval_server/problem/prob" + problemid + ".ppm");
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
    question_data problem(problem_id, playerid, size, selectable, cost_select, cost_change, block);
    return problem;
}
// 🍣🍣🍣  パーサ  🍣🍣🍣
void pcserver::parse(question_data const& problem, answer_list const& answer)
{
    // outputに本番サーバと同じ情報、outerrにそれ以外を吐く
    output << "under construction!!\n";
}


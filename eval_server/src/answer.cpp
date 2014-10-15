#include "answer.hpp"

// 🍣🍣🍣  解答変換  🍣🍣🍣
void Answer::convert(std::string const& s)
{
    answer_type al;
    answer_atom a;
    std::istringstream ss;
    ss.str(s);

    int nl; // 選択回数
    ss >> nl;
    for(int i = 0; i < nl; i++) {
        std::string pos;
        ss >> pos;
        point_type ipos{std::stoi(pos.substr(0, 1), nullptr, 16), std::stoi(pos.substr(1, 1), nullptr, 16)};
        if(ipos.x == -1 || ipos.y == -1) {
            outerr << "ERROR: illegal position: \"" + pos + "\"\n";
            return;
        } else
            a.position = ipos;

        int nx; // 交換回数
        ss >> nx;
        std::string move("");
        ss >> move;
        a.actions = std::move(move);

        al.list.push_back(a);
    }
    final_answer = al;
    outerr << "STATUS: submitted answer was loaded successfully\n" << std::endl;
    sane = true;
}


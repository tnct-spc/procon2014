#include "answer.hpp"

// 🍣🍣🍣  解答変換  🍣🍣🍣
void Answer::convert(std::string const& s)
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
            return;
        } else
            a.position = ipos;

        int nx; // 交換回数
        ss >> nx;
        std::string move;
        ss >> move;
        for(int j = 0; j < nx; j++) {
            a.actions.push_back(move[j]);
        }

        al.push_back(a);
    }
    final_answer = al;
    outerr << "STATUS: answer loaded successfully\n" << std::endl;
    sane = true;
}


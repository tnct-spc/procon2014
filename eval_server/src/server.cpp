#include "server.hpp"

std::string pcserver::parse(std::vector< std::vector<int> > const &problem) // いずれ変わる
{
    // 解答を受け取って結果を吐く

    std::stringstream output;
    bool acceptable = true;
    int nl; // num of line
    std::cin >> nl;

    for(int i = 0; i < nl; i++) {
        std::string pos; // 選択画像位置
        std::vector<int> ipos; // 選択画像位置をアレしたもの(2D)
        std::cin >> pos;
        for(int j = 0; j < pos.size(); j++) { // x, y
            auto pos2int = [j, pos, &acceptable](std::string pos) -> int {
                if('0' <= pos[j] && pos[j] <= '9')
                    return pos[j] - '0';
                else if('A' <= pos[j] && pos[j] <= 'F')
                    return pos[j] - 'A' + 10;
                else e
                    output << "ERROR: 選択画像位置が理解出来ません(\"" + pos + "\")\n";
                    acceptable = false;
                    return -1;
                }
            };
            if(acceptable)
                ipos.push_back(pos2int(pos));
        }

        int nx; // num of exchange
        std::string xchg;
        std::cin >> nx >> xchg;
        if(nx != xchg.size()) {
            output << "ERROR: 移動回数の指定が間違っています\n";
            acceptable = false;
        }

        for(int j = 0; j < nx; j++) {
            auto move = [&problem,](char x) -> bool { 
            switch(x) {
                case 'R':
                    break;
                case 'L':
                    break;
                case 'U':
                    break;
                case 'D':
                    break;
                default:
                    output << "ERROR: 移動" + xchg.substr(j, 1) + "\n";
                    acceptable = false;
                    break;
            }
            move(xchg[j]);
        }
    }
    if(acceptable)
        output << "ACCEPTED " << "XX" << std::endl;

    return output.str();
}


﻿#include <iostream>
#include <fstream>
#include <random>
#include "slide_algorithm/algorithm.hpp"
#include "data_type.hpp"
#include "test_tool.hpp"

int main(int argc, char* argv[])
{
    // スコープ長いけれど……
    std::string const output_filename = "report_movement";

    if(argc != 2 || argc != 4)\
    if(argc != 2 && argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " 試行回数 [分割数タテ 分割数ヨコ]" << std::endl;
        //std::quick_exit(-1);
        return -1;
    }

    bool const is_fixed  = argc == 4;
    int  const try_num   = std::atoi(argv[1]);
    auto const split_num = is_fixed ? std::make_pair(std::atoi(argv[3]), std::atoi(argv[2])) : std::make_pair(-1, -1);

    std::mt19937 mt;
    std::uniform_int_distribution<int> select_dist(1, 20); // 選択可能回数生成器
    std::uniform_int_distribution<int> size_dist  (2, 16); // サイズ生成器
    std::uniform_int_distribution<int> cost_dist  (1, 50); // コスト生成器

    for(int n=0; n<try_num; ++n)
    {
        int const problem_id = 0; // 仮
        std::string const player_id = "0"; //仮
        std::pair<int,int> const size = is_fixed ? split_num : std::make_pair(size_dist(mt), size_dist(mt));
        int const selectable = select_dist(mt);
        int const cost_select = cost_dist(mt);
        int const cost_change = cost_dist(mt);

        std::vector<std::vector<point_type>> block(
            size.second,
            std::vector<point_type>(size.first, {-1, -1})
            );

        std::uniform_int_distribution<int> x_dist(0, size.first  - 1);
        std::uniform_int_distribution<int> y_dist(0, size.second - 1);
        for(int i=0; i<size.second; ++i)
        {
            for(int j=0; j<size.first; ++j)
            {
                int x, y;
                do
                {
                    x = x_dist(mt);
                    y = y_dist(mt);
                }
                while(!(block[y][x] == point_type{-1, -1}));

                block[y][x] = point_type{j, i};
            }
        }

        // テストデータ発行
        question_data question{
            problem_id,
            player_id,
            size,
            selectable,
            cost_select,
            cost_change,
            block
            };

        // 実行から評価まで
        algorithm algo;
        algo.reset(question);     // テストデータのセット

        // エミュレータ起動
        test_tool::emulator emu(question);

        // 評価保存先ファイルを開き，問題データを書き込む
        std::ofstream ofs(output_filename + std::to_string(n) + ".csv");
        ofs << "[Question]\n";
        for(auto const& line : block)
        {
            for(auto const& elem : line)
            {
                ofs << R"(")" << elem.x << "," << elem.y << R"(", )";
            }
            ofs << "\n";
        }
        ofs << "\n";

        ofs << "[Answer]\n";
        ofs << "回数,間違い位置数,コスト\n";

        int counter = 0;
        while(auto first_result = algo.get())  // 解答が受け取れる間ループ
        {
            // 評価
            auto const evaluate = emu.start(first_result.get());

            // 書出
            ofs << counter+1 << "," <<evaluate.wrong << "," << evaluate.cost << "\n";

            ++counter; // テストなので解答回数をカウントする必要あるかなと
        }
    }

    return 0;
}


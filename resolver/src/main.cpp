﻿#include <iostream>
#include <array>
#include <vector>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"
#include "pixel_sorter.hpp"
#include "ppm_reader.hpp"
#include "algorithm.hpp"
#include "gui.hpp"
#include "network.hpp"

#include <sort_algorithm/yrange2.hpp>
#include <sort_algorithm/genetic.hpp>

class analyzer : boost::noncopyable
{
public:
    explicit analyzer()
        : netclient_()
    {
    }
    virtual ~analyzer() = default;

    question_data operator() (int const problem_id, std::string const& player_id)
    {
        question_raw_data raw;
#if 1
        // ファイルから
        std::string const path("prob01.ppm");
        raw = reader_.from_file(path);
#else
        // ネットワーク通信から
        std::string const data = netclient_.get_problem(01).get();
        raw = reader_.from_data(data);
#endif

        // 手作業用のウィンドウの作成
        auto future = gui::make_mansort_window(raw, "test");

        // yrangeなどの実行
        question_data formed = {
            problem_id,
            player_id,
            raw.split_num,
            raw.selectable_num,
            raw.cost.first,
            raw.cost.second,
            sorter_(raw)
        };

        //手作業のデータはこっちで受ける
        auto man_resolved = future.get();

        // TODO: ここで，sorter_(raw)の結果がイマイチなら，
        // man_resolvedが結果を置き換える可能性を考慮．

        return std::move(formed);
    }

private:
    ppm_reader reader_;
    network::client netclient_;
    pixel_sorter<yrange2> sorter_;
};

// 問題の並び替えパズル自体は，人間が行うほうがいいかもしれない．

int main()
{
    analyzer analyze;
    auto const data = analyze(1, "test token");

    algorithm algorithm;
    algorithm(data);

    return 0;
}

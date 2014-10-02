// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS

#include <iostream>
#include <array>
#include <vector>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"
#include "pixel_sorter.hpp"
#include "ppm_reader.hpp"
#include "splitter.hpp"
#include "algorithm.hpp"
#include "algorithm_2.hpp"
#include "gui.hpp"
#include "network.hpp"

#include <sort_algorithm/yrange2.hpp>
#include <sort_algorithm/yrange5.hpp>
#include <sort_algorithm/genetic.hpp>
#include <sort_algorithm/Murakami.hpp>

class analyzer : boost::noncopyable
{
public:
    explicit analyzer(int const problem_id, std::string const& player_id)
        : netclient_(), first_(true), problem_id_(problem_id), player_id_(player_id)
    {
    }
    virtual ~analyzer() = default;

    question_data operator() ()
    {
        if(first_)
        {
            first_ = false;
            return first_process(problem_id_, player_id_);
        }
        else
        {
            // TODO: yrange5の結果を待つなど
            return question_data{};
        }
    }

private:
    question_data first_process(int const problem_id, std::string const& player_id)
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
        question_data formed = {
            problem_id,
            player_id,
            raw.split_num,
            raw.selectable_num,
            raw.cost.first,
            raw.cost.second,
            std::vector<std::vector<point_type>>()
        };

        // 2次元画像に分割
        auto splitted = splitter().split_image(raw);

        // yrangeなどの実行
        auto sorter_resolve = sorter_(raw, splitted);
        formed.block = std::move(sorter_resolve);

        // TODO: yrange5の実行

        return std::move(formed);
    }

    int const problem_id_;
    std::string const player_id_;

    bool first_;
    ppm_reader reader_;
    network::client netclient_;
    pixel_sorter<Murakami> sorter_;
};

question_data convert_block(question_data const& data)
{
    auto res = data.clone();

    for(int i = 0; i < data.size.second; ++i)
    {
        for(int j = 0; j < data.size.first; ++j)
        {
            auto const& target = data.block[i][j];
            res.block[target.y][target.x] = point_type{j, i};
        }
    }

    return res;
}

// 問題の並び替えパズル自体は，人間が行うほうがいいかもしれない．

int main()
{
    auto const ploblemid = 1;
    auto const token     = "3935105806";

    analyzer        analyze(ploblemid, token);
    algorithm       algo;
    network::client client;

    // 原画像推測部
    auto const suggested = analyze();
    auto const converted = convert_block(suggested);

    // 手順探索部
    algo.reset(converted);
    auto const answer = algo.get();

    // 1発目の提出
    auto submit_result = client.submit(ploblemid, token, answer.get());
    std::cout << submit_result.get() << std::endl;

//    // 手作業用のウィンドウの作成
//    auto future = gui::make_mansort_window(splitted, "test");

    // 2発目以降の提出など(yrange5)
    auto const second_suggested = analyze();
    algo.reset(convert_block(second_suggested));
    auto const second_answer = algo.get();

    auto submit_second_result = client.submit(ploblemid, token, second_answer.get());
    std::cout << submit_second_result.get() << std::endl;

    return 0;
}


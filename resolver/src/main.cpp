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
#include "gui.hpp"
#include "network.hpp"

#include <sort_algorithm/yrange2.hpp>
#include <sort_algorithm/genetic.hpp>
#include <sort_algorithm/Murakami.hpp>
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
        splitter sp;
        auto splitted = sp.split_image(raw);

        // 手作業用のウィンドウの作成
        auto future = gui::make_mansort_window(splitted, "test");

        // yrangeなどの実行
        question_data formed = {
            problem_id,
            player_id,
            raw.split_num,
            raw.selectable_num,
            raw.cost.first,
            raw.cost.second,
            sorter_(raw, splitted)
        };

        // TODO: ここで，sorter_(raw)の結果がイマイチなら，
        // man_resolvedが結果を置き換える可能性を考慮．

		if (formed.block.size() == 0){
        //手作業のデータはこっちで受ける
        auto man_resolved = future.get();

			formed.block = man_resolved;
		}

        return std::move(formed);
    }

private:
    ppm_reader reader_;
    network::client netclient_;
    pixel_sorter<yrange2> sorter_;
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
    analyzer analyze;
    auto const data = analyze(1, "test token");
    auto const converted = convert_block(data);

    algorithm algo;
    algo.reset(converted);

    auto const answer = algo.get();
    // 送信処理をしたり，結果を見て再実行(algo.get())したり．

    return 0;
}

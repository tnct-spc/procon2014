// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS

// Macro: Program Settings
#define ENABLE_NETWORK_IO 1

#include <iostream>
#include <deque>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"
#include "pixel_sorter.hpp"
#include "ppm_reader.hpp"
#include "splitter.hpp"
#include "algorithm.hpp"
#include "algorithm_2.hpp"
#include "gui.hpp"
#include "network.hpp"
#include "test_tool.hpp"

#include <sort_algorithm/yrange2.hpp>
#include <sort_algorithm/yrange5.hpp>
#include <sort_algorithm/genetic.hpp>
#include <sort_algorithm/Murakami.hpp>

class position_manager : boost::noncopyable
{
public:
    typedef question_data position_type;

    position_manager() = default;
    virtual ~position_manager() = default;

    template<class T>
    void add(T && pos)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        items_.push_back(std::forward<T>(pos));
    }

    position_type get()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto res = items_.front();
        items_.pop_front();
        return res;
    }

    bool empty()
    {
        return items_.empty();
    }

private:
    std::mutex mutex_;
    std::deque<position_type> items_;
};

class analyzer : boost::noncopyable
{
public:
    explicit analyzer(int const problem_id, std::string const& player_id)
        : client_(), problem_id_(problem_id), player_id_(player_id)
    {
    }
    virtual ~analyzer() = default;

    void operator() (position_manager& manager)
    {
        // 問題文の入手
        raw_data_ = get_raw_question();
        data_     = get_skelton_question(raw_data_);
        
        // 2次元画像に分割
        split_image_ = splitter().split_image(raw_data_);
        // 原画像推測部
        // TODO: yrangeなどの実行
        auto sorter_resolve = sorter_(raw_data_, split_image_);
        //data_.block = std::move(sorter_resolve);
        data_.block = sorter_resolve;
        if(!data_.block.empty())manager.add(convert_block(data_)); // 解答

        // TODO: yrange5の実行(並列化)
        
        // 画面表示をいくつか(yrange/Murakmi/yrange5/algo2 etc.)
        std::vector<boost::shared_ptr<gui::impl::MoveWindow>> windows;
		if (!data_.block.empty())windows.push_back(
            gui::make_mansort_window(split_image_, sorter_resolve, "Murakami")
            );
		if (!data_.block.empty())windows.push_back(
            gui::make_mansort_window(split_image_, sorter_resolve, "Murakami")
            );
		if (data_.block.empty() && data_.block.empty()){//どっちもダメだった時
			windows.push_back(
				gui::make_mansort_window(split_image_, "Yor are the sorter!!! Sort this!")
				);
		}
        boost::thread th(
            [&]()
            {
                // futureリストでvalidを巡回し，閉じられたWindowから解とする
                while(!windows.empty())
                {
                    for(auto it = windows.begin(); it != windows.end();)
                    {
                        auto res = gui::get_result(*it);
                        if(res)
                        {
							if (res.get().size() == 0)
							{
								//TODO:man sorterの答え受け取るのはこのタイミングが良いのではないだろうか．
								continue;
							}
                            data_.block = res.get();
                            manager.add(convert_block(data_)); // 解答

                            it = windows.erase(it);
                        }
                        else ++it;
                    }
                }
            });

        gui::wait_all_window();

        th.join();
        
    }

    std::string submit(answer_type const& ans) const
    {
        auto submit_result = client_.submit(problem_id_, player_id_, ans);
        return submit_result.get();
    }

private:
    question_raw_data get_raw_question() const
    {
#if ENABLE_NETWORK_IO
        // ネットワーク通信から
        std::string const data = client_.get_problem(problem_id_).get();
        return ppm_reader().from_data(data);
#else
        // ファイルから
        std::string const path("prob01.ppm");
        return ppm_reader().from_file(path);
#endif
    }

    question_data get_skelton_question(question_raw_data const& raw) const
    {
        question_data formed = {
            problem_id_,
            player_id_,
            raw.split_num,
            raw.selectable_num,
            raw.cost.first,
            raw.cost.second,
            std::vector<std::vector<point_type>>()
        };

        return formed;
    }

    question_data convert_block(question_data const& data) const
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

    int const problem_id_;
    std::string const player_id_;

    question_raw_data raw_data_;
    question_data         data_;
    split_image_type  split_image_;

    mutable network::client client_;
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

int main()
{
    auto const ploblemid = 0;
    auto const token     = "3935105806";

    analyzer         analyze(ploblemid, token);
    algorithm_2      algo;
    position_manager manager;

    boost::thread thread(boost::bind(&analyzer::operator(), &analyze, std::ref(manager)));

    while(true)
    {
        if(!manager.empty())
        {
            // 手順探索部
            auto question = manager.get();
            algo.reset(question);
            auto const answer = algo.get();

            if(answer) // 解が見つかった
            {
#if ENABLE_NETWORK_IO
                // TODO: 前より良くなったら提出など(普通にいらないかも．提出前に目grepしてるわけだし)
                auto result = analyze.submit(answer.get());
                std::cout << "Submit Result: " << result << std::endl;
#else
                test_tool::emulator emu(question);
                auto result = emu.start(answer.get());
                std::cout << "Wrong: " << result.wrong << std::endl;
                std::cout << "Cost : " << result.cost  << std::endl;
                std::cout << "---" << std::endl;
#endif
            }
        }
    }

    thread.join();
    
    return 0;
}


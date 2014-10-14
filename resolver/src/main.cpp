﻿// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS

// Macro: Program Settings
#define ENABLE_NETWORK_IO 1

#include <iostream>
#include <deque>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
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
    explicit analyzer(
        std::string const& server_addr,
        int const problem_id, std::string const& player_id,
        bool const is_yrange, bool const is_murakami)
        : client_(server_addr), problem_id_(problem_id), player_id_(player_id),
          is_yrange_(is_yrange), is_murakami_(is_murakami)
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
        auto image_comp = sorter_.image_comp(raw_data_, split_image_);

        // 原画像推測部
        yrange2 yrange2_(raw_data_, image_comp);
        Murakami murakami_(raw_data_, image_comp);

        // GUI Threadの起動
        gui::manager gui_thread(
            [this, &manager](std::vector<std::vector<point_type>> const& data)
            {
                // 回答としてマーク -> 回答ジョブに追加
                auto clone = data_.clone();
                clone.block = data;
                manager.add(convert_block(clone));
            });

        boost::optional<boost::thread> y_thread;
        boost::optional<boost::thread> m_thread;

        // YRange2 -> YRange5 Thread
        if(is_yrange_)
        {
            y_thread = boost::thread(
                [&, this]()
                {
                    // YRange2
                    auto yrange2_resolve = yrange2_();
                    if (!yrange2_resolve.empty())
    		        {
                        // Shoot
                        auto clone = data_.clone();
                        clone.block = yrange2_resolve[0].points;
                        manager.add(convert_block(clone));
    
                        // GUI
    			        for (int y2 = yrange2_resolve.size() - 1; y2 >= 0; --y2)
    			        {
                            gui_thread.push_back(
                                boost::bind(gui::make_mansort_window, split_image_, yrange2_resolve.at(y2).points, "yrange2")
                                );
    			        }
    		        }
                    
                    // YRange5
                    auto yrange5_resolve = yrange5(raw_data_, image_comp)(yrange2_.sorted_matrix());
                    if (!yrange5_resolve.empty())
    		        {
                        // GUI
    			        for (int y5 = yrange5_resolve.size() - 1; y5 >= 0; --y5)
    			        {
                            gui_thread.push_back(
                                boost::bind(gui::make_mansort_window, split_image_, yrange5_resolve.at(y5).points, "yrange5")
                                );
    			        }
                    }
                });
        }

        // Murakami Thread
        if(is_murakami_)
        {
            m_thread = boost::thread(
                [&]()
                {
                    // Murakami
                    auto murakami_resolve = murakami_()[0].points;
                    if(!murakami_resolve.empty())gui_thread.push_back(
                        boost::bind(gui::make_mansort_window, split_image_, murakami_resolve, "Murakami")
                    );
                });
        }

        gui_thread.push_back(
            boost::bind(gui::make_mansort_window, split_image_, "Yor are the sorter!!! Sort this!")
            );

        // 各Threadの待機
        if(y_thread) y_thread->join();
        if(m_thread) m_thread->join();
        gui_thread.wait_all_window();
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
    bool const is_yrange_;
    bool const is_murakami_;

    question_raw_data raw_data_;
    question_data         data_;
    split_image_type  split_image_;

    mutable network::client client_;
    pixel_sorter<yrange5> sorter_;
};

void submit_func(question_data question, analyzer const& analyze)
{
    algorithm algo;
    algorithm_2 algo2;
    algo.reset(question);

    auto const answer = algo.get();
    if(answer) // 解が見つかった
    {
#if ENABLE_NETWORK_IO
        // TODO: 前より良くなったら提出など(普通にいらないかも．提出前に目grepしてるわけだし)
        auto result = analyze.submit(answer.get());
        std::cout << "Submit Result: " << result << std::endl;

	// FIXME: 汚いしセグフォする
	int wrong_number = std::stoi(result.substr(result.find(" ")));
	if(wrong_number == 0)
	{
        algo2.reset(question);
		auto const answer = algo2.get();
		if (answer)
		{
			auto result = analyze.submit(answer.get());
			std::cout << "Submit Result: " << result << std::endl;
			std::cout << "勝った！" << std::endl;
		}
	}
#else
        test_tool::emulator emu(question);
        auto result = emu.start(answer.get());
        std::cout << "Wrong: " << result.wrong << std::endl;
        std::cout << "Cost : " << result.cost  << std::endl;
        std::cout << "---" << std::endl;
#endif
    }
}

int main(int const argc, char const* argv[])
{
    std::string server_addr;
    int         problemid;
    auto const  token = "3935105806";
    bool        is_yrange;
    bool        is_murakami;

    try
    {
        namespace po = boost::program_options;
        po::options_description opt("option");
        opt.add_options()
            ("help,h"    , "produce help message")
            ("yrange,y"  , "process yrange2/5")
            ("muramami,m", "process murakami")
            ("server,s"  , po::value<std::string>(&server_addr), "(require)set server ip address")
            ("problem,p" , po::value<int>(&problemid)          , "(require)set problem_id");

        po::variables_map argmap;
        po::store(po::parse_command_line(argc, argv, opt), argmap);
        po::notify(argmap);

        if(argmap.count("help") || !argmap.count("server") || !argmap.count("problem"))
        {
            std::cout << opt << std::endl;
            return 0;
        }

        is_yrange   = argmap.count("yrange");
        is_murakami = argmap.count("murakami");
    }
    catch(boost::program_options::error_with_option_name const& e)
    {
        std::cout << e.what() << std::endl;
        std::quick_exit(-1);
    }

    analyzer         analyze(server_addr, problemid, token, is_yrange, is_murakami);
    position_manager manager;

    boost::thread thread(boost::bind(&analyzer::operator(), &analyze, std::ref(manager)));
    boost::thread_group submit_threads;

    while(thread.joinable())
    {
        if(!manager.empty())
        {
            auto question = manager.get();

            // 手順探索部
            submit_threads.create_thread(
                boost::bind(submit_func, question, boost::ref(analyze))
                );
        }
    }

    thread.join();
    submit_threads.join_all();
    
    return 0;
}


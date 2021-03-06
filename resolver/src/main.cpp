﻿// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS

// Macro: Program Settings
#define ENABLE_NETWORK_IO 1
#define ENABLE_SAVE_IMAGE 0

#include <iostream>
#include <deque>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/timer/timer.hpp>
#include <boost/range/algorithm.hpp>
#include "data_type.hpp"
#include "image_comparator.hpp"
#include "ppm_reader.hpp"
#include "splitter.hpp"
#include "slide_algorithm/algorithm.hpp"
#include "slide_algorithm/algorithm_2.hpp"
#include "slide_algorithm/truncater.hpp"
#include "slide_algorithm/charles.hpp"
#include "gui.hpp"
#include "network.hpp"
#include "test_tool.hpp"

#include <image_algorithm/yrange2.hpp>
#include <image_algorithm/yrange5.hpp>
#include <image_algorithm/genetic.hpp>
#include <image_algorithm/Murakami.hpp>

class position_manager : boost::noncopyable
{
public:
    typedef question_data position_type;

    position_manager() = default;
    virtual ~position_manager() = default;

    template<class T>
    void add(T && pos, bool const auto__ = false)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        items_.push_back(std::forward<T>(pos));
        auto_.push_back(auto__);
    }

    std::pair<position_type,bool> get()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto res = items_.front(); auto res_b = auto_.front();
        items_.pop_front();
        auto_.pop_front();
        return std::make_pair(res, res_b);
    }

    bool empty()
    {
        return items_.empty();
    }

private:
    std::mutex mutex_;
    std::deque<position_type> items_;
    std::deque<bool>          auto_;
};

class analyzer : boost::noncopyable
{
public:
    explicit analyzer(
        boost::shared_ptr<network::client> const& network_client,
        int const problem_id, std::string const& player_id,
        bool const is_auto, bool const is_blur
#if ENABLE_SAVE_IMAGE
        ,std::string const& dir_path = "./saved_image"
#endif
        )
        : client_(network_client), problem_id_(problem_id), player_id_(player_id)
        , is_auto_(is_auto), is_blur_(is_blur)
#if ENABLE_SAVE_IMAGE
        , dir_path_(dir_path)
#endif
    {
    }
    virtual ~analyzer() = default;

    void operator() (position_manager& manager)
    {
        splitter sp;

        // 問題文の入手
        raw_data_ = get_raw_question();
        data_     = get_skelton_question(raw_data_);

        // 2次元画像に分割
        split_image_ = sp.split_image(raw_data_);

        // compare用の画像を作成
        auto const arranged_split = is_blur_ ? sp.split_image_gaussianblur(split_image_) : split_image_;
        auto const image_comp    = comparator_   .image_comp(raw_data_, arranged_split);
		auto const image_comp_dx = comparator_dx_.image_comp(raw_data_, arranged_split);

        // 原画像推測部
        yrange2 yrange2_(raw_data_, image_comp);
        Murakami murakami_(raw_data_, image_comp,true);

        // GUI Threadの起動
        gui::manager gui_thread(
            [this, &manager](std::vector<std::vector<point_type>> const& data)
            {
                // 回答としてマーク -> 回答ジョブに追加
                auto clone = data_.clone();
                clone.block = data;
                manager.add(convert_block(clone));
            });

        // YRange2 -> YRange5 Thread
        boost::thread y_thread(
            [&, this]()
            {
                // YRange2
                auto yrange2_resolve = yrange2_();
                if (!yrange2_resolve.empty())
		        {
                    // Shoot
                    if(is_auto_)
                    {
                        is_auto_ = false;

                        std::cout << "Yrange2 Auto" << std::endl;
                        auto clone = data_.clone();
                        clone.block = yrange2_resolve[0].points;
                        manager.add(convert_block(clone), true);
                    }

                    // GUI
			        for (int y2 = yrange2_resolve.size() - 1; y2 >= 0; --y2)
			        {
                        gui_thread.push_back(
                            boost::bind(gui::make_mansort_window, split_image_, yrange2_resolve.at(y2).points, "yrange2")
                            );
			        }
		        }
                
                // YRange5
				if (raw_data_.split_num.first > 3 || raw_data_.split_num.second > 3){
					auto yrange5_resolve = yrange5(raw_data_, image_comp)(yrange2_.sorted_matrix());
					if (!yrange5_resolve.empty())
					{
						// Shoot
						if(is_auto_)
						{
							is_auto_ = false;

                            std::cout << "Yrange5 Auto" << std::endl;
							auto clone = data_.clone();
							clone.block = yrange5_resolve[0].points;
							manager.add(convert_block(clone), true);
						}

						// GUI
						for (int y5 = yrange5_resolve.size() - 1; y5 >= 0; --y5)
						{
							gui_thread.push_back(
								boost::bind(gui::make_mansort_window, split_image_, yrange5_resolve.at(y5).points, "yrange5")
								);
						}
					}
				}
            });

        // Murakami Thread
        boost::thread m_thread(
            [&]()
            {
                // Murakami
		    	std::cout << "村上モード" << std::endl;
                auto murakami_resolve = murakami_()[0].points;
				std::vector<std::vector<point_type>> murakami_dx_resolve, murakami_w_resolve, murakami_dx_w_resolve;
				if (!murakami_resolve.empty())
				{
					if(is_auto_)
					{
						is_auto_ = false;

                        std::cout << "Murakami Auto" << std::endl;
						auto clone = data_.clone();
						clone.block = murakami_resolve;
						manager.add(convert_block(clone), true);
					}
					gui_thread.push_back(boost::bind(gui::make_mansort_window, split_image_, murakami_resolve, "Murakami"));
                }
				if (murakami_resolve.empty()){
					std::cout << "デラックス村上モード" << std::endl;
					Murakami murakami_dx(raw_data_, image_comp_dx, true);
					murakami_dx_resolve = murakami_dx()[0].points;
					if (!murakami_dx_resolve.empty())gui_thread.push_back(boost::bind(gui::make_mansort_window, split_image_, murakami_dx_resolve, "Murakami_dx"));
					if (murakami_dx_resolve.empty()){
						std::cout << "W村上モード" << std::endl;
						Murakami murakami_w(raw_data_, image_comp, false);
						murakami_w_resolve = murakami_w()[0].points;
						if (!murakami_w_resolve.empty())gui_thread.push_back(boost::bind(gui::make_mansort_window, split_image_, murakami_w_resolve, "Murakami_w"));
						if (murakami_w_resolve.empty()){
							std::cout << "デラックスW村上モード" << std::endl;
							Murakami murakami_dx_w(raw_data_, image_comp_dx, false);
							murakami_dx_w_resolve = murakami_w()[0].points;
							if (!murakami_dx_w_resolve.empty())gui_thread.push_back(boost::bind(gui::make_mansort_window, split_image_, murakami_dx_w_resolve, "Murakami_dx_w"));
						}
					}
				}
            });

        gui_thread.push_back(
            boost::bind(gui::make_mansort_window, split_image_, "Yor are the sorter!!! Sort this!")
            );

        // 各Threadの待機
        y_thread.join();
        m_thread.join();
        gui_thread.wait_all_window();
    }

    int calc_cost(answer_type const& ans) const
    {
        int cost = (timer_.elapsed().wall / 1000000000) * 100;
        cost += ans.list.size() * data_.cost_select;
        boost::for_each(
            ans.list,
            [&,this](answer_atom const& move)
            {
                cost += move.actions.size() * data_.cost_change;
            });
 
        return cost;
    }

    std::string submit(answer_type const& ans, bool is_auto = false) const
    {
        std::lock_guard<std::mutex> lock(submit_mutex_);
        int const cost = calc_cost(ans);

	if(is_auto)
	{
            auto submit_result = client_->submit(problem_id_, player_id_, ans);
            return submit_result.get();
	}
	
        char input;
        std::cout << "★Submit: Cost = " << cost << " [Y/n]★";
        std::cin.get(input);

        if(input == '\n' || input == 'y' || input == 'Y')
        {
            auto submit_result = client_->submit(problem_id_, player_id_, ans);
            return submit_result.get();
        }

        return std::string();
    }

private:
    question_raw_data get_raw_question() const
    {
#if ENABLE_NETWORK_IO
        // ネットワーク通信から
        std::string const data = client_->get_problem(problem_id_).get();
        timer_.start();
#if ENABLE_SAVE_IMAGE
        std::ofstream ofs((boost::format("%s/prob%02d.ppm") % dir_path_ % problem_id_).str(), std::ios::binary);
        ofs << data;
        ofs.close();
#endif
        return ppm_reader().from_data(data);
#else
        // ファイルから
        std::string const path("prob01.ppm");
        timer_.start();
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
    bool is_auto_;
    bool const is_blur_;

    question_raw_data raw_data_;
    question_data         data_;
    split_image_type  split_image_;
	split_image_type  split_image_gaussianblur;

    mutable boost::shared_ptr<network::client> client_;
    image_comparator    comparator_;
	image_comparator_dx comparator_dx_;

    // 送信用mutex
    mutable boost::timer::cpu_timer timer_;
    mutable std::mutex submit_mutex_;

#if ENABLE_SAVE_IMAGE
    mutable std::string dir_path_;
#endif
};

void submit_func(question_data question, analyzer const& analyze, bool is_auto)
{
    algorithm algo;
    truncater talgo;

    algorithm_2 algo2;
	charles algo3;

    boost::optional<answer_type> algo_answer;
    boost::optional<answer_type> talgo_answer;
    boost::optional<answer_type> answer;

    auto algo_thread = boost::thread(
        [&algo, &algo_answer, &question]() {
            algo.reset(question);
            algo_answer = algo.get();
        }
    );

    auto talgo_thread = boost::thread(
        [&talgo, &talgo_answer, &question]() {
            talgo.reset(question);
            talgo_answer = talgo.get();
        }
    );

    algo_thread.join();
    talgo_thread.join();

    int algo_score = algo_answer->get_score(question.cost_select, question.cost_change);
    int talgo_score = talgo_answer->get_score(question.cost_select, question.cost_change);
    if (algo_score < talgo_score) {
        answer = algo_answer;
    } else {
        answer = talgo_answer;
    }

    if(answer) // 解が見つかった
    {
#if ENABLE_NETWORK_IO
        // TODO: 前より良くなったら提出など(普通にいらないかも．提出前に目grepしてるわけだし)
		std::cout << "Submit! Wait 5 sec" << std::endl;

        std::string result;

        result = analyze.submit(answer.get(), is_auto);
        std::cout << "Submit Result: " << result << std::endl;

        if(result.find("ACCEPTED") == std::string::npos)
        {
            std::cout << "Submit Result is not \"ACCEPTED\"" << std::endl;
            return;
        }

	// FIXME: 汚いしセグフォする//セグフォはしなくなったらしい
	int wrong_number = std::stoi(result.substr(result.find(" ")));
	if (wrong_number == 0)
	{
		algo2.reset(question);
		auto const answer = algo2.get();
		if (algo2.overlimitcheck()){
			if (answer)
			{
				result = analyze.submit(answer.get());
				std::cout << "Submit Result: " << result << std::endl;
				std::cout << "勝った！" << std::endl;
			}
		}
		else{
			std::cout << "algo2は答えを返しませんでした" << std::endl;
		}

		algo3.reset(question);
		auto const answer2 = algo3.get();
		if (algo3.overlimitcheck()){
			if (answer2)
			{
				result = analyze.submit(answer2.get());
				std::cout << "Submit Result 2 : " << result << std::endl;
				std::cout << "さらに勝った！" << std::endl;
			}
		}
		else{
			std::cout << "algo3は答えを返しませんでした" << std::endl;
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
	int         problemid = 0;
    auto const  token = "1";//"3935105806";
    bool        is_auto;
    bool        is_blur;
    std::string url_format;
#if ENABLE_SAVE_IMAGE
    std::string save_dir;
#endif

    try
    {
        namespace po = boost::program_options;
        po::options_description opt("option");
        opt.add_options()
            ("help,h"    , "produce help message")
            ("auto,a"    , "auto submit flag")
            ("blur,b"    , "gaussian blur to image")
#if ENABLE_SAVE_IMAGE
            ("save_dir"  , po::value<std::string>(&save_dir)   , "(require)set image dir to save")
#endif
#if ENABLE_NETWORK_IO
            ("prob_format,f", po::value<std::string>(&url_format)->default_value("/problem/prob%02d.ppm"), "set problem format(ex. /problem/prob%02d.ppm)")
            ("server,s"  , po::value<std::string>(&server_addr), "(require)set server ip address")
            ("problem,p" , po::value<int>(&problemid)          , "(require)set problem_id")
#endif
            ;

        po::variables_map argmap;
        po::store(po::parse_command_line(argc, argv, opt), argmap);
        po::notify(argmap);

#if ENABLE_NETWORK_IO
        if(argmap.count("help") || !argmap.count("server") || !argmap.count("problem"))
#else
        if(argmap.count("help"))
#endif
        {
            std::cout << opt << std::endl;
            return 0;
        }

        is_auto = argmap.count("auto");
        is_blur = argmap.count("blur");
    }
    catch(boost::program_options::error_with_option_name const& e)
    {
        std::cout << e.what() << std::endl;
        std::exit(-1);
    }

    auto             network_client = boost::make_shared<network::client>(server_addr, url_format/*, submit_path*/);

#if ENABLE_SAVE_IMAGE
    if(save_dir.empty()) save_dir = "./saved_image";
    analyzer         analyze(network_client, problemid, token, is_auto, is_blur, save_dir);
#else
    analyzer         analyze(network_client, problemid, token, is_auto, is_blur);
#endif
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
                boost::bind(submit_func, question.first, boost::ref(analyze), question.second)
                );
        }
    }

    thread.join();
    submit_threads.join_all();
    
    return 0;
}


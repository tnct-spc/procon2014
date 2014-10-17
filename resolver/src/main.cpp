// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS

// Macro: Program Settings
#define ENABLE_NETWORK_IO 1
#define ENABLE_SAVE_IMAGE 1

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
#include "charles.hpp"
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
        , dir_path_(dir_path), saved_num_(0)
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
        auto const image_comp = sorter_.image_comp(raw_data_, arranged_split);
		auto const image_comp_dx = sorter_dx.image_comp(raw_data_, arranged_split);

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
                        auto clone = data_.clone();
                        clone.block = yrange2_resolve[0].points;
                        manager.add(convert_block(clone));
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
				if (raw_data_.split_num.first >= 2 || raw_data_.split_num.first >= 2){
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
				if (!murakami_resolve.empty())gui_thread.push_back(boost::bind(gui::make_mansort_window, split_image_, murakami_resolve, "Murakami"));
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

    std::string submit(answer_type const& ans) const
    {
        auto submit_result = client_->submit(problem_id_, player_id_, ans);
        return submit_result.get();
    }

private:
    question_raw_data get_raw_question() const
    {
#if ENABLE_NETWORK_IO
        // ネットワーク通信から
        std::string const data = client_->get_problem(problem_id_).get();
#if ENABLE_SAVE_IMAGE
        std::ofstream ofs((boost::format("%s/prob%02d.ppm") % dir_path_ % saved_num_++).str(), std::ios::binary);
        ofs << data;
        ofs.close();
#endif
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
    bool const is_auto_;
    bool const is_blur_;

    question_raw_data raw_data_;
    question_data         data_;
    split_image_type  split_image_;
	split_image_type  split_image_gaussianblur;

    mutable boost::shared_ptr<network::client> client_;
    pixel_sorter<yrange5> sorter_;
	pixel_sorter<yrange2> sorter_dx;

#if ENABLE_SAVE_IMAGE
    mutable std::string dir_path_;
    mutable int         saved_num_;
#endif
};

void submit_func(question_data question, analyzer const& analyze)
{
    algorithm algo;
    algorithm_2 algo2;
	charles algo3;
    algo.reset(question);

    auto const answer = algo.get();
    if(answer) // 解が見つかった
    {
#if ENABLE_NETWORK_IO
        // TODO: 前より良くなったら提出など(普通にいらないかも．提出前に目grepしてるわけだし)
		std::cout << "Submit! Wait 5 sec" << std::endl;

        std::string result;
        do{
            result = analyze.submit(answer.get());
            std::cout << "Submit Result: " << result << std::endl;
        }
        while(result == "ERROR");

	// FIXME: 汚いしセグフォする//セグフォはしなくなったらしい
	int wrong_number = std::stoi(result.substr(result.find(" ")));
	if(wrong_number == 0)
	{
        algo2.reset(question);
		auto const answer = algo2.get();
		if (answer)
		{
			do{
				result = analyze.submit(answer.get());
				std::cout << "Submit Result: " << result << std::endl;
			}
			while(result == "ERROR");
			std::cout << "勝った！" << std::endl;
		}

		algo3.reset(question);
		auto const answer2 = algo3.get();
		if (answer2)
		{
			do{
				result = analyze.submit(answer2.get());
				std::cout << "Submit Result 2 : " << result << std::endl;
			}
			while(result == "ERROR");
			std::cout << "さらに勝った！" << std::endl;
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
    auto const  token = "3935105806";
    bool        is_auto;
    bool        is_blur;
#if ENABLE_SAVE_IMAGE
    std::string url_format;
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
                boost::bind(submit_func, question, boost::ref(analyze))
                );
        }
    }

    thread.join();
    submit_threads.join_all();
    
    return 0;
}


// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS

#include <string>
#include <fstream>
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
		if (first_)
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

	void get_probname(std::string const& in_probname){
		probname = in_probname;
	}
private:
	std::string probname;

	question_data first_process(int const problem_id, std::string const& player_id)
	{
		question_raw_data raw;
#if 1
		// ファイルから
		std::string const path(probname + ".ppm");
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

		// 手作業用のウィンドウの作成
		auto future = gui::make_mansort_window(splitted, "exchange");

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

	for (int i = 0; i < data.size.second; ++i)
	{
		for (int j = 0; j < data.size.first; ++j)
		{
			auto const& target = data.block[i][j];
			res.block[target.y][target.x] = point_type{ j, i };
		}
	}

	return res;
}

// 問題の並び替えパズル自体は，人間が行うほうがいいかもしれない．

question_data submain(std::string probname)
{
	auto const ploblemid = 1;
	auto const token = "3935105806";

	analyzer        analyze(ploblemid, token);
	algorithm_2       algo;
	network::client client;

	analyze.get_probname(probname);

	// 原画像推測部
	auto const suggested = analyze();
	//auto const converted = convert_block(suggested);
	return suggested;

	// 手順探索部
	//algo.reset(converted);
	auto const answer = algo.get();

	// 1発目の提出
	auto submit_result = client.submit(ploblemid, token, answer.get());
	std::cout << submit_result.get() << std::endl;

	// 2発目以降の提出など(yrange5)
	//auto const second_suggested = analyze();
	//algo.reset(convert_block(second_suggested));
	//auto const second_answer = algo.get();
	//
	//auto submit_second_result = client.submit(ploblemid, token, second_answer.get());
	//std::cout << submit_second_result.get() << std::endl;

	std::cout << "■■■algorythm end■■■" << std::endl;
	return suggested;
}

int main()
{
	while (1){
		std::ifstream ifs("testdata.txt");
		std::string str;
		std::vector<std::string> column;
		while (getline(ifs, str)) {
			std::cout << str << std::endl;
			boost::algorithm::split(column, str, boost::is_any_of(",")); // カンマで分割
			for (int i = 0; i < column.size(); i++){
				std::cout << "[" << column[i] << "]";
			}
		}
		std::string probname;
		std::cout << "prob number:";
		std::cin >> probname;
		auto const& answer = submain(probname);
		//比較
		
		std::ifstream fans(probname + ".ans");
		std::string sans;
		fans >> sans;
		std::vector<std::string> anscolumn;
		boost::algorithm::split(anscolumn, sans, boost::is_any_of(",")); // カンマで分割
		int change = 0;
		
		int ansy, ansx, resy, resx;
		std::cout << answer.size.second << "," << answer.size.first << std::endl;
		for (int couy = 0; couy < answer.size.second; couy++){
			if (change == 1)break;
			for (int coux = 0; coux < answer.size.first; coux++){
				std::cout << "couy=" << couy << " coux=" << coux << std::endl;
				ansy = answer.block[couy][coux].y;
				ansx = answer.block[couy][coux].x;
				std::cout << "ansy=" << ansy << " ansx=" << ansx << std::endl;
				resy = std::atoi(anscolumn[(ansy*answer.size.first + ansx) * 2].c_str());
				resx = std::atoi(anscolumn[(ansy*answer.size.first + ansx) * 2 +1].c_str());
				std::cout << "resy=" << resy << " resx=" << resx << std::endl;
				std::cout << std::endl;
				if (!(couy == resy&&coux == resx)){
					change = 1;
					break;
				}
			}
		}
		
		/*
		for (int i = 0; i < anscolumn.size(); i++){
			if (std::atoi(anscolumn[i].c_str()) != answer.block[i / answer.size.first][i % answer.size.first].y){
				std::cout << "std::atoi(anscolumn[i].c_str())=" << std::atoi(anscolumn[i].c_str()) << std::endl;
				std::cout << "answer.block[i / answer.size.first][i % answer.size.first].y=" << answer.block[i / answer.size.first][i % answer.size.first].y << std::endl;
				std::cout << "i / answer.size.first=" << i / answer.size.first << std::endl;
				std::cout << "i % answer.size.first=" << i % answer.size.first << std::endl;
				std::cout << "answer.size.first=" << answer.size.first << std::endl;
				std::cout << "answer.size.first=" << answer.size.first << std::endl;
				change = 1;
				break;
			}
			if (std::atoi(anscolumn[i].c_str()) != answer.block[i / answer.size.first][i % answer.size.first].x){
				change = 1;
				break;
			}
		}
		*/
		
		if (change == 1){
			std::cout << "failed" << std::endl;
		}
		else{
			std::cout << "success" << std::endl;
		}
		
	}
}

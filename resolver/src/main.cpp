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
#include <shlwapi.h>
#pragma comment(lib, "ShLwApi.Lib")

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
		std::string const path("ppm/"+probname + ".ppm");
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
	network::client client;

	analyze.get_probname(probname);

	// 原画像推測部
	auto const suggested = analyze();
	//auto const converted = convert_block(suggested);
	return suggested;
}

int main()
{
	while (1){
		std::cout << "      name       y*x  Murakami   yrange2" << std::endl;
		std::ifstream ifs("testdata.txt");
		std::string str;
		std::vector<std::string> column;
		while (getline(ifs, str)) {
			boost::algorithm::split(column, str, boost::is_any_of(",")); // カンマで分割
			for (int i = 0; i < column.size(); i++){
				std::cout << std::setw(10) << column[i];
				
			}
			std::cout << std::endl;
		}
		std::string probname;
		int mode = 0;
		//std::cout << "If you want run all, you write \"@sort_all\"\nIf you want run all failed, you write \"@sort_all_failed\"\nIf you want run all success, you write \"@sort_all_success\"\n";
		while (1){
			std::cout << "prob name:";
			std::cin >> probname;
			if (probname == "@sort_all"){
				mode = 1;
				break;
			}
			if (probname == "@sort_all_failed"){
				mode = 2;
				break;
			}
			if (probname == "@sort_all_success"){
				mode = 3;
				break;
			}
			//存在確認
			int ppm = PathFileExists(("ppm/" + probname + ".ppm").c_str());
			int ans = PathFileExists(("ans/" + probname + ".ans").c_str());
			if (ppm == 1 && ans == 1){
				break;
			}
			else if (ppm == 1 && ans != 1){
				std::cout << "\"ans/" << probname << ".ans\"" << "does not exist" << std::endl;
			}
			else if (ppm != 1 && ans == 1){
				std::cout << "\"ppm/" << probname << ".ppm\"" << "does not exist" << std::endl;
			}
			else{
				std::cout << "\"ans/" << probname << ".ans\"" << "does not exist" << std::endl;
				std::cout << "\"ppm/" << probname << ".ppm\"" << "does not exist" << std::endl;
			}
		}
		//std::cout << "mode=" << mode << std::endl;
		auto const& answer = submain(probname);
		//比較
		
		std::ifstream fans("ans/"+probname + ".ans");
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
				resy = std::atoi(anscolumn[(couy*answer.size.first + coux) * 2].c_str());
				resx = std::atoi(anscolumn[(couy*answer.size.first + coux) * 2 +1].c_str());
				std::cout << "resy=" << resy << " resx=" << resx << std::endl;
				std::cout << std::endl;
				if (!(ansy == resy&&ansx == resx)){
					change = 1;
					break;
				}
			}
		}
		int how = 0;
		if (change == 1){
			how = 1;
			std::cout << "failed" << std::endl;
		}
		else{
			std::cout << "success" << std::endl;
		}
		//書き込み
		std::ifstream kifs("testdata.txt");
		std::string kstr;
		std::vector<std::vector<std::string>> kcolumn;
		kcolumn.resize(256);
		for (int i = 0; i < 256; i++)
		{
			kcolumn[i].resize(256);
		}
		
		int kcount = 0;
		while (getline(kifs, kstr)) {
			boost::algorithm::split(kcolumn[kcount], kstr, boost::is_any_of(",")); // カンマで分割
			kcount++;
		}
		int check = 0;
		std::ofstream kkifs("testdata.txt");
		for (int i = 0; i < kcount; i++)
		{
			if (kcolumn[i][0] == probname){
				check = 1;
				kkifs << probname << ",";
				kkifs << answer.size.second << "*" << answer.size.first << ",";
				if (how == 1){
					kkifs << "failed,";
				}
				else{
					kkifs << "success,";
				}
				kkifs << ",";
			}
			else{
				for (int j = 0; j < 4; j++)
				{
					kkifs << kcolumn[i][j] << ",";
				}
			}
			kkifs << std::endl;
		}
		if (check == 0){
			kkifs << probname << ",";
			kkifs << answer.size.second << "*" << answer.size.first << ",";
			if (how == 1){
				kkifs << "failed,";
			}
			else{
				kkifs << "success,";
			}
			kkifs << ",";
			kkifs << std::endl;
		}	
	}
}

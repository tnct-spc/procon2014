#ifndef RESOLVER_ALGORITHM_2_HPP
#define RESOLVER_ALGORITHM_2_HPP

#include <vector>
#include <iostream>
#include <boost/optional.hpp>
#include "data_type.hpp"

class heap
{
private:
	int y, x;
	long long int pos = 0;
	long long int size = 0;
	int size_million = 1;
	std::vector<long long int> heaptable;
	std::vector<int> cost;
	std::vector< std::vector<int> > history;
	std::vector<int>history_limit;
	std::vector< std::vector<int> > table;
public:
	heap();//コンストラクタ
	void setup(int y, int x);
	~heap();//デストラクタ
	void pop(int *in_cost, int in_table[], std::vector<int> &in_history, int *history_limit);
	void push(int *in_cost, int in_table[], std::vector<int> &in_history, int *history_limit);
};

class algorithm_2
{
public:
	int debug_c = 0;
	typedef answer_type return_type;

	algorithm_2();
	~algorithm_2();

	auto get()->boost::optional<return_type>;
	void reset(question_data const& data);


	//持ってきた関数
	//search(int y, int  x, int *start);//コンストラクタ
	//~search();//デストラクタ
	int puzzle(std::vector<int> &out_history, int &out_cost);//探索

private:
	boost::optional<question_data> data_;
	std::vector<std::vector<point_type>> matrix;
	int width;
	int height;
	int cost_select;
	int cost_change;
	int *table;


	//持ってきた変数
	int goal = 10;
	int sentaku = 10;
	int coukan = 10;
	int *agotable;
	int size_million_history = 1;
	int size_million_keiro = 1;
	heap harray;
	int cost;
	std::vector<int> history;
	std::vector<int> subhistory;
	std::vector<int> keiro;
	//int *table;
	int history_limit = 0;
	int subhistory_limit = 0;
	int keiro_count = 0;
	int size;
	int ysize;
	int xsize;
	//持ってきた関数
	void prescanning();
	void scanning(int i, int j, int i_pre, int j_pre, int URDL);
	void shorting();
	void reverse(int*);
};

#endif

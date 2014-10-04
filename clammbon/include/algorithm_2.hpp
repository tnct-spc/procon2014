#ifndef RESOLVER_ALGORITHM_2_HPP
#define RESOLVER_ALGORITHM_2_HPP

#include <vector>
//#include <iostream>
//#include <boost/optional.hpp>
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
	heap();
	~heap();

	void setup(int y, int x);
	void pop(int *in_cost, int in_table[], std::vector<int> &in_history, int *history_limit);
	void push(int *in_cost, int in_table[], std::vector<int> &in_history, int *history_limit);
};

class algorithm_2
{
public:
	typedef answer_type return_type;

	algorithm_2();
	~algorithm_2();

	auto get()->boost::optional<return_type>;
	void reset(question_data const& data);

private:
	boost::optional<question_data> data_;
	int cost_select;
	int cost_change;
	int *table;
	int *sub_table;
	int goal = 1;
	int sentaku = 1;
	int coukan = 1;
	int size_million_history = 1;
	int size_million_keiro = 1;
	heap harray;
	int cost;
	std::vector<int> history;
	std::vector<int> sub_history;
	std::vector<int> keiro;
	int history_limit = 0;
	int sub_history_limit = 0;
	int keiro_count = 0;
	int size;
	int y_size;
	int x_size;

	void prescanning();
	void scanning(int i, int j, int i_pre, int j_pre, int URDL);
	void shorting();
	void reverse(int*);
	int puzzle(std::vector<int> &out_history, int &out_cost);//íTçı
};

#endif

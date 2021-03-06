#ifndef RESOLVER_ALGORITHM_2_HPP
#define RESOLVER_ALGORITHM_2_HPP

#include <vector>
#include <iomanip>
#include "data_type.hpp"
#include <set>

//収納配列
class heap
{
public:
	heap();
	~heap();

	//初期化
	void setup(const int* y, const int* x);
	//→挿入
	void pop(int *in_cost, std::vector<int> &in_table, std::vector<int> &in_history, int *in_history_limit);
	//←取出
	void push(int *out_cost, std::vector<int> &out_table, std::vector<int> &out_history, int *out_history_limit);
	//vectorの解放
	void end();
private:
	int yx, y, x;
	int pos = 0;
	int size = 0;
	int sizemaxcount = 1;
	std::vector<int> heaptable;
	std::vector<int> cost;
	std::vector< std::vector<int> > history;
	std::vector<int>history_limit;
	std::vector< std::vector<int> > table;
	std::vector<bool> LIST_OC;

	class NODE{
	public:
		//コンストラクタ
		NODE(std::vector<int> iti, int n) :hairetu(iti), pos(n){}

		//変数
		std::vector<int> hairetu;
		int pos;

		friend inline bool operator< (NODE const& lhs, NODE const& rhs)
		{
			return (lhs.hairetu == rhs.hairetu) ? lhs.hairetu < rhs.hairetu : lhs.hairetu < rhs.hairetu;
		}
	};
	std::set<heap::NODE> NODE_;
};


class algorithm_2{
public:
	typedef answer_type return_type;
	boost::optional<question_data> data_;

	algorithm_2();
	~algorithm_2();
	//初期化
	void reset(question_data const& data);
	//実行
	auto get()->boost::optional<return_type>;
	bool overlimitcheck();
private:

	heap harray;

	int goal = 10;
	int sentaku = 10;
	int coukan = 10;
	//1,1,1		ノーマル
	//10,5,3	選択が短いのを作れた
	//10 8 4
	//10 14 6

	int size_y, size_x, size;
	int cost_s, cost_c;
	int cost_slimit;
	std::vector<int> table;
	int cost;
	int keiro_count = 0;
	std::vector<int> history;
	std::vector<int> keiro;
	int history_limit = 0;
	int sizemaxhistory = 1;
	int sizemaxkeiro = 1;
	int G, S, C;
	int sub_history_limit = 0;//use shorting
	std::vector<int> sub_history;//use shorting
	std::vector<int> root1;//use shorting
	std::vector<int> root2;//use shorting
	int sizemaxroot1 = 1;//use shorting
	int sizemaxroot2 = 1;//use shorting
	bool OVER_LIMIT = false;


	//順番に探索を開始する
	void prescanning();
	//探索
	void scanning(int i, int j, int i_pre, int j_pre, int URDL);
	//経路を短くする
	void shorting();
	//経路反転
	void reverse(int* x);
};

#endif

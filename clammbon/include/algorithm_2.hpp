#ifndef RESOLVER_ALGORITHM_2_HPP
#define RESOLVER_ALGORITHM_2_HPP

#include <vector>
#include <iomanip>
#include "data_type.hpp"
#include <set>

//û”[”z—ñ
class heap
{
public:
	heap();
	~heap();

	//‰Šú‰»
	void setup(const int* y, const int* x);
	//¨‘}“ü
	void pop(int *in_cost, std::vector<int> &in_table, std::vector<int> &in_history, int *in_history_limit);
	//©æo
	void push(int *out_cost, std::vector<int> &out_table, std::vector<int> &out_history, int *out_history_limit);
	//vector‚Ì‰ğ•ú
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
		//ƒRƒ“ƒXƒgƒ‰ƒNƒ^
		NODE(std::vector<int> iti, int n) :hairetu(iti), pos(n){}

		//•Ï”
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
	//‰Šú‰»
	void reset(question_data const& data);
	//Às
	auto get()->boost::optional<return_type>;
	bool overlimitcheck();
private:

	heap harray;

	int goal = 10;
	int sentaku = 10;
	int coukan = 10;
	//1,1,1		ƒm[ƒ}ƒ‹
	//10,5,3	‘I‘ğ‚ª’Z‚¢‚Ì‚ğì‚ê‚½
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


	//‡”Ô‚É’Tõ‚ğŠJn‚·‚é
	void prescanning();
	//’Tõ
	void scanning(int i, int j, int i_pre, int j_pre, int URDL);
	//Œo˜H‚ğ’Z‚­‚·‚é
	void shorting();
	//Œo˜H”½“]
	void reverse(int* x);
};

#endif

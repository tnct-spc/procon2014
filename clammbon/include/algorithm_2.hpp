#ifndef RESOLVER_ALGORITHM_2_HPP
#define RESOLVER_ALGORITHM_2_HPP

#include <vector>
#include <iomanip>
#include "data_type.hpp"

//���[�z��
class heap
{
public:
	heap();
	~heap();

	//������
	void setup(const int* y, const int* x);
	//����o
	void pop(int *in_cost, std::vector<int> &in_table, std::vector<int> &in_history, int *in_history_limit);
	//���}��
	void push(int *in_cost, std::vector<int> &in_table, std::vector<int> &in_history, int *in_history_limit);

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
};



//�A���S���Y��
class algorithm_2
{
public:
	typedef answer_type return_type;

	algorithm_2();
	~algorithm_2();

	//������
	void reset(question_data const& data);

	//���s
	auto get() -> boost::optional<return_type>;

private:
	heap harray;
	boost::optional<question_data> data_;
	const int goal = 10;
	const int sentaku = 10;
	const int coukan = 10;
	//1,1,1		�m�[�}��
	//10,5,3	�I�����Z���̂���ꂽ
	//10 8 4
	//10 14 6
	int cost;
	int size;
	int y_size;
	int x_size;
	int selectcost;
	int changecost;
	int sizemaxhistory = 1;
	int sizemaxkeiro = 1;
	int history_limit = 0;
	int sub_history_limit = 0;
	int keiro_count = 0;
	int G, S, C;
	std::vector<int> table;
	std::vector<int> sub_table;
	std::vector<int> history;
	std::vector<int> sub_history;
	std::vector<int> keiro;
	std::vector<int> root1;//use shorting
	std::vector<int> root2;//use shorting
	int sizemaxroot1 = 1;
	int sizemaxroot2 = 1;

	//�A���S���Y���̎��s
	void puzzle(std::vector<int> &out_history, int &out_cost);
	//���ԂɒT�����J�n����
	void prescanning();
	//�T��
	void scanning(int i, int j, int i_pre, int j_pre, int URDL);

	//�o�H��Z������
	void shorting();
	//�o�H���]
	void reverse(int* x);
};

#endif

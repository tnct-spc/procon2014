#ifndef RESOLVER_ALGORITHM_2_HPP
#define RESOLVER_ALGORITHM_2_HPP

#include <vector>
#include <iomanip>
#include "data_type.hpp"
#include <boost/container/set.hpp>
#include <boost/timer/timer.hpp>
#include <set>

//���[�z��
class heap
{
public:
	heap();
	~heap();

	//������
	void setup(const int* y, const int* x);
	//���}��
	bool pop(int *in_cost, std::vector<int> &in_table, std::vector<int> &in_history, int *in_history_limit, int *in_oya);
	//����o
	bool push(int *out_cost, std::vector<int> &out_table, std::vector<int> &out_history, int *out_history_limit, int *out_oya);
	//�e�̃A���T�[��f��
	void pushanswer(int oyanum);
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
	std::vector<int> oya;

	bool posover = false;
	boost::timer::cpu_timer timer; // ���Ԍv��
	int push_count=0;
	class NODE{
	public:
		//�R���X�g���N�^
		NODE(std::vector<int> iti, int n) :hairetu(iti), pos(n){}

		//�ϐ�
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
	//������
	void reset(question_data const& data);
	//���s
	auto get()->boost::optional<return_type>;
private:

	heap harray;
	//std::vector<std::vector<uint8_t>> start_distance;
	//std::vector<std::vector<uint8_t>> goal_distance;

	int goal = 1;
	int sentaku = 0;
	int coukan = 0;
	//1,1,1		�m�[�}��
	//10,5,3	�I�����Z���̂���ꂽ
	//10 8 4
	//10 14 6

	int size_y, size_x, size;
	int cost_s, cost_c;
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
	bool do_exchange;
	bool limitter;
	bool none;
	bool first=true;
	bool kirikae=false;

	//���ԂɒT�����J�n����
	void prescanning();
	//�T��
	void scanning(int i, int j, int i_pre, int j_pre, int URDL);
	//�o�H��Z������
	void shorting();
	//�o�H���]
	void reverse(int* x);
	//�e
	int oya = -1;
};

#endif
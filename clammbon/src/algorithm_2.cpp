//////////////////////////
//�ŗD��T���A���S���Y��//
//////////////////////////

//�C���N���[�h�t�@�C��
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "algorithm_2.hpp"

//�R���X�g���N�^
algorithm_2::algorithm_2()
{
}

//�f�X�g���N�^
algorithm_2::~algorithm_2()
{
	delete[] table;
}

auto algorithm_2::get() -> boost::optional<return_type>
{
	std::cout << "algorithm_2  start" << std::endl;
	// �摜
	matrix = data_->block;

	// ���ƍ���
	width = data_->size.first;
	height = data_->size.second;

	// �R�X�g�ƃ��[�g
	cost_select = data_->cost_select;
	cost_change = data_->cost_change;

	//�e�[�u���ɓ����
	table = new int[width*height];//y*x�T�C�Y�̃e�[�u�������

	//�\��
	std::cout << "width=" << width << ",height=" << height << std::endl;
	for (int y = 0; height > y; y++){
		for (int x = 0; width > x; x++){
			std::cout << "(" << matrix[y][x].y << "," << matrix[y][x].x << ")";
			table[y*width + x] = matrix[y][x].y*width + matrix[y][x].x;
		}
		std::cout<<std::endl;
	}
	for (int y = 0; height > y; y++){
		for (int x = 0; width > x; x++){
			std::cout << table[y*width + x] << ",";
		}
		std::cout << std::endl;
	}
	//stop
	int i;
	std::cin >> i;
	return 0;
	//return pimpl_->get();
}

void algorithm_2::reset(question_data const& data)
{
	data_ = data.clone();
	//pimpl_->reset(data);
}
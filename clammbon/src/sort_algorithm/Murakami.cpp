#include <iostream>
#include <boost/format.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <data_type.hpp>
#include <sort_algorithm/compare.hpp>
#include <sort_algorithm/adjacent.hpp>
#include <sort_algorithm/Murakami.hpp>

Murakami::Murakami(question_raw_data const& data, compared_type const& comp)
	: data_(data), comp_(comp)
{
}

std::vector<std::vector<std::vector<point_type>>> Murakami::operator() (){
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	//sort_compare();//sorted_comparation�쐬
	make_sorted_comparation();
	//block_type block(height, std::vector<point_type>(width, { -1, -1 }));
	//�傫����[height * width][height][width]�Œ��g��x,y = -1�ŏ�����
	std::vector<std::vector<std::vector<point_type>>> block_list(
		height * width,
		std::vector<std::vector<point_type>>(
		height,
		std::vector<point_type>(
		width,
		{
		-1, -1
		}
		)

		)
		);

		//�u���b�N�̍���ɕ����摜��z�u���郋�[�v
		for (int i = 0; i < height; i++){
			for (int j = 0; j < width; j++){
				block_list[i * width + j][0][0].y = i;
				block_list[i * width + j][0][0].x = j;
			}
		}
		int a;
		while (block_list.size() != 1){//���C�����[�v
			a = block_list.size();

			block_combination best_block_combination;//��Ԃ���block�̑g�ݍ��킹�����Ƃ���
			best_block_combination.score = std::numeric_limits<int_fast64_t>::min();
			for (auto i : block_list){//�u���b�N���m���r���郋�[�v
				for (auto j : block_list){
					if (i == j) continue;
					bool momo = false;
					if ((i[0][0].x == 2 && i[0][0].y == 2 && j[0][0].x == 0 && j[0][0].y == 1) || (j[0][0].x == 2 && j[0][0].y == 2 && i[0][0].x == 0 && i[0][0].y == 1))momo = true;

					block_combination b = eval_block(i, j);
					if (best_block_combination.score < b.score){
						best_block_combination = b; //TODO �����l����������ǂ�����
					}
				}
			}

			if (best_block_combination.score == std::numeric_limits<int_fast64_t>::min())throw std::runtime_error("�u���b�N�]���\���̂���ł�");
			block_type combined_block = combine_block(best_block_combination);//�u���b�N����������
			boost::remove_erase_if(block_list, [best_block_combination](block_type it){//block_list���猋������O�̃u���b�N������
				return (it == best_block_combination.block1 || it == best_block_combination.block2);
			});
			block_list.push_back(combined_block);//���������̂�����
			std::cout << block_list.size() << std::endl;
		}

		for (int i = 0; i < height; i++){
			for (int j = 0; j < width; j++){
				std::cout << block_list[0][i][j].x << "," << block_list[0][i][j].y << "|";
			}
			std::cout << "\n";
		}
		return block_list;

}
Murakami::block_combination Murakami::eval_block(block_type block1, block_type block2){
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	auto const in_range = [height, width](int y, int x){
		if (x >= 0 && x < width && y >= 0 && y < height){
			return true;
		}
		else{
			return false;
		}
	};
	auto const exists = [height, width](int y, int x, block_type b){
		if (x >= 0 && x < width && y >= 0 && y < height){
			if (b[y][x].x != -1 || b[y][x].y != -1){
				return true;
			}
			else{
				return false;
			}
		}
		else{
			return false;
		}
	};
	int_fast64_t best_block_c = std::numeric_limits<int_fast64_t>::min();
	int best_shift_i;
	int best_shift_j;
	for (int i = -height; i < height * 2 - 1; i++){
		//std::cout << "hi ";
		for (int j = -width; j < width * 2 - 1; j++){
			bool confliction = false;
			int_fast64_t block_c = 0;
			bool empty_block_c = true;
			int_fast64_t rank1_num = 0;//�L���X�g���ʓ|����������int_fast64_t��
			for (int k = 0; k < height; k++){
				for (int l = 0; l < width; l++){
					if (exists(k - i, l - j, block2)){
						if (block1[k][l].x != -1 && block1[k][l].y != -1 && block2[k - i][l - j].x != -1 && block2[k - i][l - j].y != -1){
							confliction = true;
							break;
						}
					}
					if (exists(k, l, block1) && !exists(k + i, l + j, block2)){
						int_fast64_t piece_c = 0;
						if (exists(k + i - 1, l + j, block2)){//��
							piece_c += eval_piece(block1[k][l], block2[k + i - 1][l + j], up);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][up][1] == block2[k + i - 1][l + j]) rank1_num++;
						}
						if (exists(k + i, l + j - 1, block2)){//��
							piece_c += eval_piece(block1[k][l], block2[k + i][l + j - 1], left);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][left][1] == block2[k + i][l + j - 1]) rank1_num++;
						}
						if (exists(k + i + 1, l + j, block2)){//��
							piece_c += eval_piece(block1[k][l], block2[k + i + 1][l + j], down);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][down][1] == block2[k + i + 1][l + j]) rank1_num++;
						}
						if (exists(k + i, l + j + 1, block2)){//�E
							piece_c += eval_piece(block1[k][l], block2[k + i][l + j + 1], right);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][right][1] == block2[k + i][l + j + 1]) rank1_num++;
						}
						//pow(piece_c, rank1_num);
						block_c += piece_c;
					}
				}
				if (confliction)break;
			}
			if (!confliction && empty_block_c == false){
				//if (best_block_c != 0){};
				rank1_num++;
				block_c *= rank1_num; //0���|����͕̂|��
				if (block_c > best_block_c){
					best_block_c = block_c;
					best_shift_i = -i;
					best_shift_j = -j;
				}
			}
		}
	}
	block_combination return_struct{
		block1,
		block2,
		best_shift_j,
		best_shift_i,
		best_block_c
	};
	return return_struct;
}
std::int_fast64_t Murakami::eval_piece(point_type p1, point_type p2, direction dir){
	int_fast64_t score = 0;
	Murakami::direction dir2;
	switch (dir)
	{
	case Murakami::up:
		dir2 = Murakami::down;
		break;
	case Murakami::right:
		dir2 = Murakami::left;
		break;
	case Murakami::down:
		dir2 = Murakami::up;
		break;
	case Murakami::left:
		dir2 = Murakami::right;
		break;
	default:
		break;
	}
	score += eval_comp_(p1, p2, dir);
	score += eval_comp_(p2, p1, dir2);
	return score;
}
std::int_fast64_t Murakami::eval_comp_(point_type p1, point_type p2, direction dir){
	int rank = 0;
	std::int_fast64_t score = 0;
	for (auto it : sorted_comparation.at(p1)[dir]){
		if (it == p2)break;
		rank++;
	}

	if (rank == 0)throw std::runtime_error("�f�Љ摜���d�����Ă��܂�");
	if (rank == 1){
		switch (dir)
		{
		case Murakami::up:
			score = comp_[p1.y][p1.x][sorted_comparation[p1][up][2].y][sorted_comparation[p1][up][2].x].up - comp_[p1.y][p1.x][p2.y][p2.x].up;
			break;
		case Murakami::right:
			score = comp_[p1.y][p1.x][sorted_comparation[p1][right][2].y][sorted_comparation[p1][right][2].x].right - comp_[p1.y][p1.x][p2.y][p2.x].right;
			break;
		case Murakami::down:
			score = comp_[p1.y][p1.x][sorted_comparation[p1][down][2].y][sorted_comparation[p1][down][2].x].down - comp_[p1.y][p1.x][p2.y][p2.x].down;
			break;
		case Murakami::left:
			score = comp_[p1.y][p1.x][sorted_comparation[p1][left][2].y][sorted_comparation[p1][left][2].x].left - comp_[p1.y][p1.x][p2.y][p2.x].left;
			break;
		default:
			break;
		}
	}
	else if (rank > 1){
		switch (dir)
		{
		case Murakami::up:
			score = comp_[p1.y][p1.x][sorted_comparation[p1][up][1].y][sorted_comparation[p1][up][1].x].up - comp_[p1.y][p1.x][p2.y][p2.x].up;
			break;
		case Murakami::right:
			score = comp_[p1.y][p1.x][sorted_comparation[p1][right][1].y][sorted_comparation[p1][right][1].x].right - comp_[p1.y][p1.x][p2.y][p2.x].right;
			break;
		case Murakami::down:
			score = comp_[p1.y][p1.x][sorted_comparation[p1][down][1].y][sorted_comparation[p1][down][1].x].down - comp_[p1.y][p1.x][p2.y][p2.x].down;
			break;
		case Murakami::left:
			score = comp_[p1.y][p1.x][sorted_comparation[p1][left][1].y][sorted_comparation[p1][left][1].x].left - comp_[p1.y][p1.x][p2.y][p2.x].left;
			break;
		default:
			break;
		}
	}
	else{

	}
	return score;
}

void Murakami::sort_compare()
{
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	const point_type init_pt{ -1, -1 };
	struct point_type_score{
		int y, x;
		int_fast64_t score;
		bool operator<(const point_type_score& right) const {
			return score == right.score ? x < right.x : score < right.score;
		}
	};
	std::map <point_type, std::vector < std::vector<point_type>>> sorted_comparation;
	std::vector<point_type_score> rank;
	std::vector<std::vector<point_type>> direct(4, (std::vector<point_type>(1)));

	//�S�Ẵs�[�X�ɑ΂��āi��ԊO���̃��[�v�j
	for (int i = 0; i < height; ++i)for (int j = 0; j < width; ++j){
		point_type now_point{ j, i };

		//�S�Ẵs�[�X��g�ݍ��킹��	##up##
		rank.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			point_type_score temp_pts{ k, l };
			if (i == k&&j == l)continue;
			temp_pts.score = comp_[i][j][k][l].up;
			rank.push_back(temp_pts);
		}
		std::sort(rank.begin(), rank.end());
		direct[0][0] = init_pt;//0�Ԗڂɂ�(-1,-1)��
		for (int c = 0; c < 10 && c < rank.size(); ++c){
			point_type temp_pt;
			temp_pt.x = rank[c].x;
			temp_pt.y = rank[c].y;
			direct[0].push_back(temp_pt);
		}


		//�S�Ẵs�[�X��g�ݍ��킹��	##down##
		rank.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			point_type_score temp_pts{ k, l };
			if (i == k&&j == l)continue;
			temp_pts.score = comp_[i][j][k][l].down;
			rank.push_back(temp_pts);
		}
		std::sort(rank.begin(), rank.end());
		direct[1][0] = init_pt;//0�Ԗڂɂ�(-1,-1)��
		for (int c = 0; c < 10 && c < rank.size(); ++c){
			point_type temp_pt;
			temp_pt.x = rank[c].x;
			temp_pt.y = rank[c].y;
			direct[1].push_back(temp_pt);
		}

		//�S�Ẵs�[�X��g�ݍ��킹��	##right##
		rank.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			point_type_score temp_pts{ k, l };
			if (i == k&&j == l)continue;
			temp_pts.score = comp_[i][j][k][l].right;
			rank.push_back(temp_pts);
		}
		std::sort(rank.begin(), rank.end());
		direct[2][0] = init_pt;//0�Ԗڂɂ�(-1,-1)��
		for (int c = 0; c < 10 && c < rank.size(); ++c){
			point_type temp_pt;
			temp_pt.x = rank[c].x;
			temp_pt.y = rank[c].y;
			direct[2].push_back(temp_pt);
		}

		//�S�Ẵs�[�X��g�ݍ��킹��	##left##
		rank.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			point_type_score temp_pts{ k, l };
			if (i == k&&j == l)continue;
			temp_pts.score = comp_[i][j][k][l].left;
			rank.push_back(temp_pts);
		}
		std::sort(rank.begin(), rank.end());
		direct[3][0] = init_pt;//0�Ԗڂɂ�(-1,-1)��
		for (int c = 0; c < 10 && c < rank.size(); ++c){
			point_type temp_pt;
			temp_pt.x = rank[c].x;
			temp_pt.y = rank[c].y;
			direct[3].push_back(temp_pt);
		}
		sorted_comparation.insert(std::make_pair(now_point, direct));
	}

	//�t�@�C�������o��
	
	std::ofstream ofs("solusions.csv", std::ios::out | std::ios::app | std::ios::ate);
	ofs << "point_type.y,point_type.x,direct,rank,point_type.y,point_type.x" << std::endl;
	for (int i = 0; i < height; ++i)for (int j = 0; j < width; ++j){
		point_type now_point{ i, j };
		for (int k = 0; k < 4; ++k){
			for (int l = 0; l < width*height - 1; ++l){
				ofs << now_point.y << "," << now_point.x << "," << k << "," << l << "," << sorted_comparation[now_point][k][l].y << "," << sorted_comparation[now_point][k][l].x << std::endl;
			}
		}
	}
	std::cout << "Output solusions done." << std::endl;
	
}

void Murakami::make_sorted_comparation(){
	auto width = data_.split_num.first;
	auto height = data_.split_num.second;
	struct point_type_score{
		point_type point;
		int_fast64_t score;
		bool operator<(const point_type_score& right) const {
			return (score < right.score);
		}
	};

	std::map<point_type, std::vector<std::vector<point_type>>> sorted_point_score_dir_point;
	for (int k = 0; k < height; k++){
		for (int l = 0; l < width; l++){
			point_type point_k_l{ l, k };
			std::vector<std::vector<point_type>> sorted_point_score_dir;
			for (int dir_l = 0; dir_l < 4; dir_l++){
				std::vector<point_type_score> sorted_point_score;
				for (int i = 0; i < height; i++){
					for (int j = 0; j < width; j++){
						point_type p{ j, i };
						point_type_score t;
						t.point = p;
						switch (dir_l)
						{
						case up:
							t.score = comp_[k][l][i][j].up;
							break;
						case right:
							t.score = comp_[k][l][i][j].right;
							break;
						case down:
							t.score = comp_[k][l][i][j].down;
							break;
						case left:
							t.score = comp_[k][l][i][j].left;
							break;
						}
						sorted_point_score.push_back(t);
					}
				}
				std::sort(sorted_point_score.begin(), sorted_point_score.end());
				std::vector<point_type> p_vec;
				for (int a = 0; a < height * width; a++){
					p_vec.push_back(sorted_point_score[a].point);
				}
				sorted_point_score_dir.push_back(p_vec);
			}
			sorted_point_score_dir_point[point_k_l] = sorted_point_score_dir;
		}

	}

	sorted_comparation = sorted_point_score_dir_point;
	
	std::ofstream ofs("solusions.csv", std::ios::out | std::ios::app | std::ios::ate);
	ofs << "point_type.x,point_type.y,direct,rank,point_type.x,point_type.y" << std::endl;
	for (int i = 0; i < height; ++i)for (int j = 0; j < width; ++j){
		point_type now_point{ i, j };
		for (int k = 0; k < 4; ++k){
			for (int l = 0; l < width*height ; ++l){
				ofs << now_point.x << "," << now_point.y << "," << k << "," << l << "," << sorted_comparation[now_point][k][l].x << "," << sorted_comparation[now_point][k][l].y << std::endl;
			}
		}
	}
	std::cout << "Output solusions done." << std::endl;
	
}
Murakami::block_type Murakami::combine_block(block_combination block_comb){
	if (block_comb.block1.size() == 0)throw std::runtime_error("�����\��u���b�N�̃T�C�Y��0�ł�,block1");
	if (block_comb.block2.size() == 0)throw std::runtime_error("�����\��u���b�N�̃T�C�Y��0�ł�,block1");
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	auto const exists = [height, width](int y, int x, block_type b){
		if (x >= 0 && x < width && y >= 0 && y < height){
			if (b[y][x].x != -1 || b[y][x].y != -1){
				return true;
			}
			else{
				return false;
			}
		}
		else{
			return false;
		}
	};
	int l_x = 0, l_y = 0;
	if (block_comb.shift_x < 0){
		l_x = -block_comb.shift_x;
	}
	if (block_comb.shift_y < 0){
		l_y = -block_comb.shift_y;
	}
	/*
	int min_x = -width;
	int max_x = width * 2 - 1;
	int min_y = -height;
	int max_y = height * 2 - 1;
	for (int i = -height; i < height * 2 - 1; i ++){
	for (int j = -width; j < width * 2 - 1; j++){
	//TODO exsist?
	//if ((block_comb.block1[i][j].x != -1 && block_comb.block1[i][j].y != -1) || (block_comb.block2[i + block_comb.shift_y][j + block_comb.shift_x].x != -1 && block_comb.block2[i + block_comb.shift_y][j + block_comb.shift_x].y != -1)){
	if (exists(i, j, block_comb.block1) || exists(i + block_comb.shift_y, j + block_comb.shift_x, block_comb.block2)){
	if (min_x > j)min_x = j;
	if (max_x < j)max_x = j;
	if (min_y > i)min_y = i;
	if (max_y < i)max_y = i;
	}
	}
	}
	*/
	std::vector<std::vector<point_type>> return_combined_block(height, std::vector<point_type>(width, { -1, -1 }));
	for (int i = -l_y; i < height * 2 - 1; i++){
		for (int j = -l_x; j < width * 2 - 1; j++){
			if (exists(i, j, block_comb.block1)){
				return_combined_block[i + l_y][j + l_x] = block_comb.block1[i][j];
			}
			else if (exists(i - block_comb.shift_y, j - block_comb.shift_x, block_comb.block2)){
				return_combined_block[i + l_y][j + l_x] = block_comb.block2[i - block_comb.shift_y][j - block_comb.shift_x];
			}
		}
	}
	return return_combined_block;
}
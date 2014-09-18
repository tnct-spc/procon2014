﻿#include <iostream>
#include <boost/format.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_map>
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
	//sort_compare();//sorted_comparation作成
	make_sorted_comparation();
	//block_type block(height, std::vector<point_type>(width, { -1, -1 }));
	//大きさが[height * width][height][width]で中身がx,y = -1で初期化
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

		//ブロックの左上に分割画像を配置するループ
		for (int i = 0; i < height; i++){
			for (int j = 0; j < width; j++){
				block_list[i * width + j][0][0].y = i;
				block_list[i * width + j][0][0].x = j;
			}
		}
		int a;
		while (block_list.size() != 1){//メインループ
			a = block_list.size();

			block_combination best_block_combination;//一番いいblockの組み合わせいれるところ
			best_block_combination.score = std::numeric_limits<int_fast64_t>::min();
			for (auto i : block_list){//ブロック同士を比較するループ
				for (auto j : block_list){
					if (i == j) continue;
					bool momo = false;
					if ((i[0][0].x == 2 && i[0][0].y == 2 && j[0][0].x == 0 && j[0][0].y == 1) || (j[0][0].x == 2 && j[0][0].y == 2 && i[0][0].x == 0 && i[0][0].y == 1))momo = true;

					block_combination b = eval_block(i, j);
					if (best_block_combination.score < b.score){
						best_block_combination = b; //TODO 同じ値があったらどうしよ
					}
				}
			}

			if (best_block_combination.score == std::numeric_limits<int_fast64_t>::min())throw std::runtime_error("ブロック評価構造体が空です");
			block_type combined_block = combine_block(best_block_combination);//ブロックを結合する
			boost::remove_erase_if(block_list, [best_block_combination](block_type it){//block_listから結合する前のブロックを消す
				return (it == best_block_combination.block1 || it == best_block_combination.block2);
			});
			block_list.push_back(combined_block);//結合したのを入れる
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
	int const b1_width = block1.size();
	int const b1_height = block1[0].size();
	int const b2_width = block2.size();
	int const b2_height = block2[0].size();
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

	auto const block1_exists = [b1_height, b1_width, block1](int y, int x){
		if (x >= 0 && x < b1_width && y >= 0 && y < b1_height){
			if (block1[y][x].x != -1 || block1[y][x].y != -1){
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
	auto const block2_exists = [b2_height, b2_width, block2](int y, int x){
		if (x >= 0 && x < b2_width && y >= 0 && y < b2_height){
			if (block2[y][x].x != -1 || block2[y][x].y != -1){
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
	for (int i = -b2_height; i < b1_height + b2_height + 1; i++){
		for (int j = -b1_width; j < b1_width + b2_width + 1; j++){
			bool confliction = false;
			int_fast64_t block_c = 0;
			bool empty_block_c = true;
			int_fast64_t rank1_num = 0;//キャストが面倒くさいからint_fast64_tで
			for (int k = 0; k < b1_height; k++){
				for (int l = 0; l < b1_width; l++){
					if (block2_exists(k - i, l - j)){
						if (block1[k][l].x != -1 && block1[k][l].y != -1 ){
							confliction = true;
							break;
						}
					}
					if (block1_exists(k, l) && !block2_exists(k + i, l + j)){
						int_fast64_t piece_c = 0;
						if (block2_exists(k + i - 1, l + j)){//上
							piece_c += eval_piece(block1[k][l], block2[k + i - 1][l + j], up);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][up][1] == block2[k + i - 1][l + j]) rank1_num++;
						}
						if (block2_exists(k + i, l + j - 1)){//左
							piece_c += eval_piece(block1[k][l], block2[k + i][l + j - 1], left);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][left][1] == block2[k + i][l + j - 1]) rank1_num++;
						}
						if (block2_exists(k + i + 1, l + j)){//下
							piece_c += eval_piece(block1[k][l], block2[k + i + 1][l + j], down);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][down][1] == block2[k + i + 1][l + j]) rank1_num++;
						}
						if (block2_exists(k + i, l + j + 1)){//右
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
				block_c *= rank1_num; //0を掛けるのは怖い
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

	if (rank == 0)throw std::runtime_error("断片画像が重複しています");
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
	std::unordered_map <point_type, std::vector < std::vector<point_type>>> sorted_comparation;
	std::vector<point_type_score> rank;
	std::vector<std::vector<point_type>> direct(4, (std::vector<point_type>(1)));

	//全てのピースに対して（一番外側のループ）
	for (int i = 0; i < height; ++i)for (int j = 0; j < width; ++j){
		point_type now_point{ j, i };

		//全てのピースを組み合わせる	##up##
		rank.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			point_type_score temp_pts{ k, l };
			if (i == k&&j == l)continue;
			temp_pts.score = comp_[i][j][k][l].up;
			rank.push_back(temp_pts);
		}
		std::sort(rank.begin(), rank.end());
		direct[0][0] = init_pt;//0番目には(-1,-1)を
		for (int c = 0; c < 10 && c < rank.size(); ++c){
			point_type temp_pt;
			temp_pt.x = rank[c].x;
			temp_pt.y = rank[c].y;
			direct[0].push_back(temp_pt);
		}


		//全てのピースを組み合わせる	##down##
		rank.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			point_type_score temp_pts{ k, l };
			if (i == k&&j == l)continue;
			temp_pts.score = comp_[i][j][k][l].down;
			rank.push_back(temp_pts);
		}
		std::sort(rank.begin(), rank.end());
		direct[1][0] = init_pt;//0番目には(-1,-1)を
		for (int c = 0; c < 10 && c < rank.size(); ++c){
			point_type temp_pt;
			temp_pt.x = rank[c].x;
			temp_pt.y = rank[c].y;
			direct[1].push_back(temp_pt);
		}

		//全てのピースを組み合わせる	##right##
		rank.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			point_type_score temp_pts{ k, l };
			if (i == k&&j == l)continue;
			temp_pts.score = comp_[i][j][k][l].right;
			rank.push_back(temp_pts);
		}
		std::sort(rank.begin(), rank.end());
		direct[2][0] = init_pt;//0番目には(-1,-1)を
		for (int c = 0; c < 10 && c < rank.size(); ++c){
			point_type temp_pt;
			temp_pt.x = rank[c].x;
			temp_pt.y = rank[c].y;
			direct[2].push_back(temp_pt);
		}

		//全てのピースを組み合わせる	##left##
		rank.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			point_type_score temp_pts{ k, l };
			if (i == k&&j == l)continue;
			temp_pts.score = comp_[i][j][k][l].left;
			rank.push_back(temp_pts);
		}
		std::sort(rank.begin(), rank.end());
		direct[3][0] = init_pt;//0番目には(-1,-1)を
		for (int c = 0; c < 10 && c < rank.size(); ++c){
			point_type temp_pt;
			temp_pt.x = rank[c].x;
			temp_pt.y = rank[c].y;
			direct[3].push_back(temp_pt);
		}
		sorted_comparation.insert(std::make_pair(now_point, direct));
	}

	//ファイル書き出し
	
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

	std::unordered_map<point_type, std::vector<std::vector<point_type>>> sorted_point_score_dir_point;
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
	/*
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
	*/
	
}
Murakami::block_type Murakami::combine_block(block_combination block_comb){
	if (block_comb.block1.size() == 0)throw std::runtime_error("結合予定ブロックのサイズが0です,block1");
	if (block_comb.block2.size() == 0)throw std::runtime_error("結合予定ブロックのサイズが0です,block2");
	int const b1_width = block_comb.block1.size();
	int const b1_height = block_comb.block1[0].size();
	int const b2_width = block_comb.block2.size();
	int const b2_height = block_comb.block2[0].size();
	auto const block1_exists = [b1_height, b1_width, block_comb](int y, int x){
		if (x >= 0 && x < b1_width && y >= 0 && y < b1_height){
			if (block_comb.block1[y][x].x != -1 || block_comb.block1[y][x].y != -1){
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
	auto const block2_exists = [b2_height, b2_width, block_comb](int y, int x){
		if (x >= 0 && x < b2_width && y >= 0 && y < b2_height){
			if (block_comb.block2[y][x].x != -1 || block_comb.block2[y][x].y != -1){
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
	std::vector<std::vector<point_type>> return_combined_block(height, std::vector<point_type>(width, { -1, -1 }));
	for (int i = -l_y; i < height * 2 - 1; i++){
		for (int j = -l_x; j < width * 2 - 1; j++){
			if (block1_exists(i,j)){
				return_combined_block[i + l_y][j + l_x] = block_comb.block1[i][j];
			}
			else if (block2_exists(i - block_comb.shift_y, j - block_comb.shift_x)){
				return_combined_block[i + l_y][j + l_x] = block_comb.block2[i - block_comb.shift_y][j - block_comb.shift_x];
			}
		}
	}
	return return_combined_block;
}

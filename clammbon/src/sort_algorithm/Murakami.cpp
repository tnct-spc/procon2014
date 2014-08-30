#include <iostream>
#include <boost/format.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
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
	sort_compare();//sorted_comparation作成
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

		while (block_list.size() == 1){//メインループ
			block_combination best_block_combination;//一番いいblockの組み合わせいれるところ
			for (auto i : block_list){//ブロック同士を比較するループ
				for (auto j : block_list){
					if (i == j) continue;
					block_combination b = eval_block(i, j);
					if (best_block_combination.score < b.score){
						best_block_combination.score = b.score; //TODO 同じ値があったらどうしよ
					}
				}
			}
			block_type combined_block = combine_block(best_block_combination);//ブロックを結合する
			boost::remove_erase_if(block_list, [best_block_combination](block_type it){//block_listから結合する前のブロックを消す
				return (it == best_block_combination.block1 || it == best_block_combination.block2);
			});
			block_list.push_back(combined_block);//結合したのを入れる
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
		for (int j = -width; j < width * 2 - 1; j++){
			bool confliction = false;
			int_fast64_t block_c = 0;
			int_fast64_t rank1_num = 0;//キャストが面倒くさいからint_fast64_tで
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
						if (exists(k + i - 1, l + j, block2)){//上
							piece_c += eval_piece(block1[k][l], block2[k + i - 1][l + j], up);
							if (sorted_comparation[block1[k][l]][up][1] == block2[k + i - 1][l + j]) rank1_num++;
						}
						if (exists(k + i, l + j - 1, block2)){//左
							piece_c += eval_piece(block1[k][l], block2[k + i][l + j - 1], left);
							if (sorted_comparation[block1[k][l]][left][1] == block2[k + i][l + j - 1]) rank1_num++;
						}
						if (exists(k + i + 1, l + j, block2)){//下
							piece_c += eval_piece(block1[k][l], block2[k + i + 1][l + j], down);
							if (sorted_comparation[block1[k][l]][down][1] == block2[k + i + 1][l + j]) rank1_num++;
						}
						if (exists(k + i, l + j + 1, block2)){//右
							piece_c += eval_piece(block1[k][l], block2[k + i][l + j + 1], right);
							if (sorted_comparation[block1[k][l]][right][1] == block2[k + i][l + j + 1]) rank1_num++;
						}
						//pow(piece_c, rank1_num);
						block_c += piece_c;
					}
				}
				if (confliction)break;
			}
			if (!confliction){
				block_c *= (rank1_num + 1); //0を掛けるのは怖い
				if (block_c > best_block_c){
					best_block_c = block_c;
					best_shift_i = i;
					best_shift_j = j;
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
	score += eval_comp_(p1, p2, dir);
	score += eval_comp_(p2, p1, dir);
	return score;
}
std::int_fast64_t Murakami::eval_comp_(point_type p1, point_type p2, direction dir){
	int pos = 0;
	std::int_fast64_t score = 0;
	for (auto it : sorted_comparation[p1][dir]){
		if (it == p2)break;
		pos++;
	}
	if (pos == 0)throw std::runtime_error("断片画像が重複しています");
	if (pos == 1){
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
	else if (pos > 1){
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

	std::map<point_type, std::vector < std::vector<point_type>>> sorted_comparation;
	
	struct point_type_score{
		uint_fast64_t score;
		int x, y;
		bool operator<(const point_type_score& right) const {
			return (score == right.score) ? x < right.x : score < right.score;
		}
	};

	std::vector<point_type_score> temp_vec;
	std::vector<std::vector<point_type> > direct(4, (4, std::vector<point_type>(width*height)));
											
	//一番外側のループ
	for (int i = 0; i < height; ++i)for (int j = 0; j < width; ++j){
		point_type_score temp_score;
		point_type temp_point;
		point_type now_point;
		now_point.x = j;
		now_point.y = i;
		
		//###up###//
		temp_vec.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			temp_score.score = comp_[i][j][k][l].up;
			temp_score.x = j;
			temp_score.y = i;
			temp_vec.push_back(temp_score);
		}
		std::sort(temp_vec.begin(), temp_vec.end());
		int d = temp_vec[0].x;
		//上から最大で10個代入
		temp_point.x = -1;
		temp_point.y = -1;
		direct[up].push_back(temp_point);
		for (int i = 0; i < 10; ++i){
			if (i >= temp_vec.size())break;
			temp_point.x = temp_vec[i].x;
			temp_point.y = temp_vec[i].y;
			direct[up].push_back(temp_point);
		}

		//###down###//
		temp_vec.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			temp_score.score = comp_[i][j][k][l].down;
			temp_score.x = j;
			temp_score.y = i;
			temp_vec.push_back(temp_score);
		}
		std::sort(temp_vec.begin(), temp_vec.end());
		//上から最大で10個代入
		temp_point.x = -1;
		temp_point.y = -1;
		direct[down].push_back(temp_point);
		for (int i = 0; i < 10; ++i){
			if (i >= temp_vec.size())break;
			temp_point.x = temp_vec[i].x;
			temp_point.y = temp_vec[i].y;
			direct[down].push_back(temp_point);
		}

		//###right###//
		temp_vec.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			temp_score.score = comp_[i][j][k][l].right;
			temp_score.x = j;
			temp_score.y = i;
		}
		temp_vec.push_back(temp_score);
		std::sort(temp_vec.begin(), temp_vec.end());
		//上から最大で10個代入
		temp_point.x = -1;
		temp_point.y = -1;
		direct[right].push_back(temp_point);
		for (int i = 0; i < 10; ++i){
			if (i >= temp_vec.size())break;
			temp_point.x = temp_vec[i].x;
			temp_point.y = temp_vec[i].y;
			direct[right].push_back(temp_point);
		}

		//###left###//
		temp_vec.clear();
		for (int k = 0; k < height; ++k)for (int l = 0; l < width; ++l){
			temp_score.score = comp_[i][j][k][l].left;
			temp_score.x = j;
			temp_score.y = i;
		}
		temp_vec.push_back(temp_score);
		std::sort(temp_vec.begin(), temp_vec.end());
		//上から最大で10個代入
		temp_point.x = -1;
		temp_point.y = -1;
		direct[left].push_back(temp_point);
		for (int i = 0; i < 10; ++i){
			if (i >= temp_vec.size())break;
			temp_point.x = temp_vec[i].x;
			temp_point.y = temp_vec[i].y;
			direct[left].push_back(temp_point);
		}
		std::map<point_type, std::vector < std::vector<point_type>>> sorted_comparation;
		sorted_comparation.insert(std::map<point_type, std::vector < std::vector<point_type>>>::value_type(now_point, direct));
	}

	std::ofstream ofs("solusions.csv", std::ios::out | std::ios::app | std::ios::ate);
	std::map<point_type, std::vector < std::vector<point_type>>>::iterator sorted_comparation_it;
	for (sorted_comparation_it = sorted_comparation.begin(); sorted_comparation_it != sorted_comparation.end(); sorted_comparation_it++){
		for (int i = 0; i < 4; ++i)for (int j = 0; j < 11; ++j){
			ofs << sorted_comparation_it->first.x << "," << sorted_comparation_it->first.y << "," << i << "," << j << "," << sorted_comparation_it->second[i][j].x << "," << sorted_comparation_it->second[i][j].y << std::endl;
		}
	}
	std::cout << "Output solusions done." << std::endl;

}

Murakami::block_type Murakami::combine_block(block_combination block){
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	for (int i = -height; i < height * 2 - 1; i ++){
		for (int j = -width; j < width * 2 - 1; j++){

		}
	}
	return block.block1;
}
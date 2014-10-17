#include <iostream>
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
#include <./gui.hpp>
#include <boost/timer.hpp>
#include <omp.h>
Murakami::Murakami(question_raw_data const& data, compared_type const& comp,bool const w_mode)
	: data_(data), comp_(comp)
{
}
/*蓮メリちゅっちゅ*/
std::vector<answer_type_y> Murakami::operator() (){
	bool const w_mode = true;
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	//sort_compare();//sorted_comparation作成
	/*
	#ifdef _OPENMP
	//OpenMPを使ったコード
	std::cout << "use openMP" << std::endl;
	#else
	//OpenMPを使わない場合のコード
	std::cout << "no use openMP" << std::endl;
	#endif
	*/
	make_sorted_comparation();
	//boost::timer t;
	std::vector<block_data_> block_list(height * width);
	unsigned int unique_number_cnt = 0;
	//ブロックの左上に分割画像を配置するループ
	for (int i = 0; i < height; i++){
		for (int j = 0; j < width; j++){
			//block_list[i * width + j][0][0].y = i;
			//block_list[i * width + j][0][0].x = j;
			block_list[i * width + j].block = { { { j, i } } };//std::vector < std::vector<point_type> > {std::vector < point_type > {point_type{ j, i } }};
			block_list[i * width + j].u_this_number = unique_number_cnt;
			unique_number_cnt++;
		}
	}
		//std::cout << "a";
		//block_combination b;
	for (int i = 0; i < block_list.size(); i++){//ブロック比較ループ改良
		block_combination b;
		for (int j = i; j < block_list.size(); j++){
			if (i == j) continue;
			b = w_mode ? eval_block(block_list[i].block, block_list[j].block) : eval_block2(block_list[i].block, block_list[j].block);
			block_list[i].score_data_[block_list[j].u_this_number] = std::move(score_data{ b.score, b.shift_x, b.shift_y });
		}
	}
	int a;//デバッグ用の変数
	try{
		while (block_list.size() != 1){//メインループ
			a = block_list.size();
			block_combination best_block_combination;//一番いいblockの組み合わせいれるところ
			best_block_combination.score = std::numeric_limits<bigint>::min();
			block_combination b;
			for (int i = 0; i < block_list.size(); i++){//ブロック比較ループ改良
				for (int j = i; j < block_list.size(); j++){
					if (i == j) continue;
					std::unordered_map<unsigned int, score_data>::iterator m_it = block_list[i].score_data_.find(block_list[j].u_this_number);
					if (m_it != block_list[i].score_data_.end()){
						//TODO ここを参照渡しにする
						b.block1 = block_list[i].block;
						b.block2 = block_list[j].block;
						b.score = block_list[i].score_data_[m_it->first].score;
						b.shift_x = block_list[i].score_data_[m_it->first].shift_x;
						b.shift_y = block_list[i].score_data_[m_it->first].shift_y;

					}
					else{
						b = w_mode ? eval_block(block_list[i].block, block_list[j].block) : eval_block2(block_list[i].block, block_list[j].block);
					}
					if (best_block_combination.score < b.score)best_block_combination = std::move(b);
				}
			}

			if (best_block_combination.score == std::numeric_limits<bigint>::min()){
				//std::cout << "本当に結合するブロックがなかったので解を出さずにMurakamiは終了します";
				throw std::runtime_error("Murakami_runtime_error");
			}
			block_type combined_block = std::move(combine_block(best_block_combination));//ブロックを結合する
			boost::remove_erase_if(block_list, [&best_block_combination](block_data_ it){//block_listから結合する前のブロックを消す
				return (it.block == best_block_combination.block1 || it.block == best_block_combination.block2);
			});
			block_data_ p_b;
			p_b.block = combined_block;
			block_list.push_back(p_b);//結合したのを入れる
			std::cout << "***" << block_list.size() << "***" << "\r" << std::flush;

			/*
			for (const auto& i : block_list){
			for (const auto& j : i.block){
			for (const auto& k : j){
			//std::cout << k.x << "," << k.y << " ";
			std::cout << boost::format("(%2d,%2d)") % k.x % k.y;
			}
			std::cout << "\n";
			}
			std::cout << "\n";
			}
			*/
			/*デバッグ画像表示
			std::vector<std::vector<std::vector<point_type>>> raw_block_list;
			for (auto const& block_it : block_list){
			raw_block_list.push_back(block_it.block);
			}
			gui::combine_show_image(data_, comp_, raw_block_list);
			*/
		}
	}catch (...){
		std::cerr << "ダメ_Murakami_だめ" << std::endl;
			//std::cout << t.elapsed() << "s経過した(Murakami内で計測)" << std::endl;
			std::vector<std::vector<point_type>>exception_array;
			std::vector<block_type> temp_array;
			for (auto hoge : block_list){
				temp_array.push_back(hoge.block);
			}
			exception_array = std::move(force_combine_block(temp_array));
			return std::vector<answer_type_y>{ { std::move(exception_array), 0, cv::Mat() } }; 

		}
		//std::cout << t.elapsed() << "s経過した(Murakami内で計測)" << std::endl;
		/*デバッグ表示
		for (int i = 0; i < height; i++){
			for (int j = 0; j < width; j++){
				std::cout << block_list[0].block[i][j].x << "," << block_list[0].block[i][j].y << "|";
			}
			std::cout << "\n";
		}
		*/
		return std::vector<answer_type_y>{ { block_list.at(0).block, 0, cv::Mat() } };

}
Murakami::block_combination Murakami::eval_block(const block_type& block1, const block_type& block2){

	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	int const b1_width = block1[0].size();
	int const b1_height = block1.size();
	int const b2_width = block2[0].size();
	int const b2_height = block2.size();
	
	auto const block1_exists = [b1_height, b1_width, &block1](int y, int x){
		return ((x >= 0 && x < b1_width && y >= 0 && y < b1_height) && (block1[y][x].x != -1 || block1[y][x].y != -1));
	};
	auto const block2_exists = [b2_height, b2_width, &block2](int y, int x){
		return ((x >= 0 && x < b2_width && y >= 0 && y < b2_height) && (block2[y][x].x != -1 || block2[y][x].y != -1));

	};
	auto const block_size_check = [b1_width, b1_height, b2_width, b2_height, width, height](int shift_y,int shift_x){
		return (std::max(b1_width, shift_x + b2_width) - std::min(0, shift_x) <= width && std::max(b1_height, shift_y + b2_height) - std::min(0, shift_y) <= height);
	};
	bigint best_block_c = std::numeric_limits<bigint>::min();

	int best_shift_i = std::numeric_limits<int>::min();
	int best_shift_j = std::numeric_limits<int>::min();
	for (int i = -b2_height -1; i <= b1_height + b2_height + 1; i++){
		for (int j = -b2_width -1; j <= b1_width + b2_width + 1; j++){
			bool confliction = false;
			if (!block_size_check(i, j)){
				continue;
			}
			bigint block_c = 0;
			bool empty_block_c = true;
			int rank1_num = 0;
			for (int k = 0; k < b1_height; k++){
				for (int l = 0; l < b1_width; l++){
					if (block2_exists(k - i, l - j) && block1_exists(k, l)){
							confliction = true;
							break;
					}else if (block1_exists(k, l) && !block2_exists(k - i, l - j)){
						//int_fast64_t piece_c = 0;
						if (block2_exists(k - i - 1, l - j)){//上
							block_c += eval_piece(block1[k][l], block2[k - i - 1][l - j], up);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][up][1] == block2[k - i - 1][l - j]) rank1_num++;
						}
						if (block2_exists(k - i, l - j - 1)){//左
							block_c += eval_piece(block1[k][l], block2[k - i][l - j - 1], left);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][left][1] == block2[k - i][l - j - 1]) rank1_num++;
						}
						if (block2_exists(k - i + 1, l - j)){//下
							block_c += eval_piece(block1[k][l], block2[k - i + 1][l - j], down);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][down][1] == block2[k - i + 1][l - j]) rank1_num++;
						}
						if (block2_exists(k - i, l - j + 1)){//右
							block_c += eval_piece(block1[k][l], block2[k - i][l - j + 1], right);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][right][1] == block2[k - i][l - j + 1]) rank1_num++;
						}
					}
				}

				if (confliction)break;
			}
			if (!confliction && !empty_block_c){
				++rank1_num;
				//block_c *= rank1_num; //0を掛けるのは怖い
				if(block_c < 0)block_c = -pow(block_c, rank1_num);
				if (block_c > 0)block_c = pow(block_c, rank1_num);
				if (block_c >= best_block_c){
					//block_size_check(i, j)
					best_block_c = block_c;
					best_shift_i = i;
					best_shift_j = j;
				}
			}
		}
	}
	/*
	if (best_shift_i == std::numeric_limits<int>::min()){
		std::cout << "結合すべきブロックがなかった" << std::endl;
	}
	*/
	block_combination return_struct{
		std::move(block1),
		std::move(block2),
		best_shift_j,
		best_shift_i,
		best_block_c
	};
	return std::move(return_struct);
}
Murakami::block_combination Murakami::eval_block2(const block_type& block1, const block_type& block2){

	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	int const b1_width = block1[0].size();
	int const b1_height = block1.size();
	int const b2_width = block2[0].size();
	int const b2_height = block2.size();

	auto const block1_exists = [b1_height, b1_width, &block1](int y, int x){
		return ((x >= 0 && x < b1_width && y >= 0 && y < b1_height) && (block1[y][x].x != -1 || block1[y][x].y != -1));
	};
	auto const block2_exists = [b2_height, b2_width, &block2](int y, int x){
		return ((x >= 0 && x < b2_width && y >= 0 && y < b2_height) && (block2[y][x].x != -1 || block2[y][x].y != -1));

	};
	auto const block_size_check = [b1_width, b1_height, b2_width, b2_height, width, height](int shift_y, int shift_x){
		return (std::max(b1_width, shift_x + b2_width) - std::min(0, shift_x) <= width && std::max(b1_height, shift_y + b2_height) - std::min(0, shift_y) <= height);
	};
	bigint best_block_c = std::numeric_limits<bigint>::min();

	int best_shift_i = std::numeric_limits<int>::min();
	int best_shift_j = std::numeric_limits<int>::min();
	for (int i = -b2_height - 1; i <= b1_height + b2_height + 1; i++){
		for (int j = -b2_width - 1; j <= b1_width + b2_width + 1; j++){
			bool confliction = false;
			if (!block_size_check(i, j)){
				continue;
			}
			bigint block_c = 0;
			bool empty_block_c = true;
			int rank1_num = 0;
			for (int k = 0; k < b1_height; k++){
				for (int l = 0; l < b1_width; l++){
					if (block2_exists(k - i, l - j) && block1_exists(k, l)){
						confliction = true;
						break;
					}
					else if (block1_exists(k, l) && !block2_exists(k - i, l - j)){
						//int_fast64_t piece_c = 0;
						if (block2_exists(k - i - 1, l - j)){//上
							block_c += eval_piece(block1[k][l], block2[k - i - 1][l - j], up);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][up][1] == block2[k - i - 1][l - j]) rank1_num++;
						}
						if (block2_exists(k - i, l - j - 1)){//左
							block_c += eval_piece(block1[k][l], block2[k - i][l - j - 1], left);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][left][1] == block2[k - i][l - j - 1]) rank1_num++;
						}
						if (block2_exists(k - i + 1, l - j)){//下
							block_c += eval_piece(block1[k][l], block2[k - i + 1][l - j], down);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][down][1] == block2[k - i + 1][l - j]) rank1_num++;
						}
						if (block2_exists(k - i, l - j + 1)){//右
							block_c += eval_piece(block1[k][l], block2[k - i][l - j + 1], right);
							empty_block_c = false;
							if (sorted_comparation[block1[k][l]][right][1] == block2[k - i][l - j + 1]) rank1_num++;
						}
					}
				}

				if (confliction)break;
			}
			if (!confliction && !empty_block_c){
				++rank1_num;
				block_c *= rank1_num; //0を掛けるのは怖い
				//if (block_c < 0)block_c = -pow(block_c, rank1_num);
				//if (block_c > 0)block_c = pow(block_c, rank1_num);
				if (block_c >= best_block_c){
					//block_size_check(i, j)
					best_block_c = block_c;
					best_shift_i = i;
					best_shift_j = j;
				}
			}
		}
	}
	/*
	if (best_shift_i == std::numeric_limits<int>::min()){
	std::cout << "結合すべきブロックがなかった" << std::endl;
	}
	*/
	block_combination return_struct{
		std::move(block1),
		std::move(block2),
		best_shift_j,
		best_shift_i,
		best_block_c
	};
	return std::move(return_struct);
}
std::int_fast64_t Murakami::eval_piece(const point_type& p1, const point_type& p2, direction dir){
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
std::int_fast64_t Murakami::eval_comp_(const point_type& p1, const point_type& p2, direction dir){
	int rank = 0;
	std::int_fast64_t score = 0;
	/*この方がかっこいいけどちょっと遅い
	std::vector<point_type>::iterator it = find(sorted_comparation[p1][dir].begin(), sorted_comparation[p1][dir].end(), p2);
	rank = it - sorted_comparation[p1][dir].begin();
	*/
	
	for (const auto& it : sorted_comparation[p1][dir]){
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

	sorted_comparation = std::move(sorted_point_score_dir_point);
}
Murakami::block_type Murakami::combine_block(const block_combination& block_comb){

	if (block_comb.block1.size() == 0)throw std::runtime_error("結合予定ブロックのサイズが0です,block1");
	if (block_comb.block2.size() == 0)throw std::runtime_error("結合予定ブロックのサイズが0です,block2");
	int const b1_width = block_comb.block1[0].size();
	int const b1_height = block_comb.block1.size();
	int const b2_width = block_comb.block2[0].size();
	int const b2_height = block_comb.block2.size();
	point_type lu, rd; //lu left_up,左上 rd right_down,右下
	lu.x = std::min(0, block_comb.shift_x);
	lu.y = std::min(0, block_comb.shift_y);
	rd.x = std::max(b1_width, block_comb.shift_x + b2_width);
	rd.y = std::max(b1_height, block_comb.shift_y + b2_height);
	block_size_type comb_block_size = rd - lu;
	auto const block1_exists = [b1_height, b1_width, &block_comb](int y, int x){
		return ((x >= 0 && x < b1_width && y >= 0 && y < b1_height) && (block_comb.block1[y][x].x != -1 || block_comb.block1[y][x].y != -1));
	};
	auto const block2_exists = [b2_height, b2_width, &block_comb](int y, int x){
		return ((x >= 0 && x < b2_width && y >= 0 && y < b2_height) && (block_comb.block2[y][x].x != -1 || block_comb.block2[y][x].y != -1));

	};
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	int l_x = 0, l_y = 0;
	if (block_comb.shift_x < 0)l_x = -block_comb.shift_x;
	if (block_comb.shift_y < 0)l_y = -block_comb.shift_y;
	std::vector<std::vector<point_type>> return_combined_block(comb_block_size.y, std::vector<point_type>(comb_block_size.x, { -1, -1 }));

	for (int i = lu.y; i < rd.y; i++){
		for (int j = lu.x; j < rd.x; j++){
			if (block1_exists(i,j)){
				return_combined_block[i + l_y][j + l_x] = block_comb.block1[i][j];
			}else if (block2_exists(i - block_comb.shift_y, j - block_comb.shift_x)){
				return_combined_block[i + l_y][j + l_x] = block_comb.block2[i - block_comb.shift_y][j - block_comb.shift_x];
			}
		}
	}

	return return_combined_block;

}

Murakami::block_type Murakami::force_combine_block(std::vector<Murakami::block_type>& block_list){
	auto max_block_it = std::max_element(block_list.begin(), block_list.end(), [](const block_type& a, const block_type& b){
		return(a.size() + a[0].size() < b.size() + b[0].size());
	});
	block_type max_block = (*max_block_it);

	boost::remove_erase_if(block_list, [&max_block](block_type it){//block_listから結合する前のブロックを消す
		return (it == max_block);
	});

	std::for_each(
		max_block.begin(), max_block.end(),
		[this](std::vector<point_type>& elem)
		{
			elem.resize(data_.split_num.first, point_type{ -1, -1 });
		});

	max_block.resize(data_.split_num.second, std::vector<point_type>(data_.split_num.first, point_type{-1,-1}));

	for (auto &i : max_block){
		for (auto &j : i){
			if (j.x == -1 || j.y == -1){
				for (auto &list_i : block_list){
					for (auto &list_j : list_i){
						for (auto &list_k : list_j){
							if (list_k.x != -1 || list_k.y != -1){
								j = list_k;
								list_k.x = -1;
								list_k.y = -1;
								goto KIRISAME_MARISA;
							}
						}
					}
				}
			}
		KIRISAME_MARISA:;
		}
	}

	return max_block;
	/*
	auto pick_up_without_it = [&block_list](const block_type){
		
	};
	for (auto i : (*max_block_it)){
		for (auto j : i){
			if (!(j.x == -1) || !(j.y == -1))continue;
			pick_up_without_it(max_block_it);
		}
	}
	*/
}

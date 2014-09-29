#ifdef _DEBUG
#include <iostream>
#include <boost/format.hpp>
#endif

#include <vector>
#include <algorithm>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <data_type.hpp>
#include <sort_algorithm/compare.hpp>
#include <sort_algorithm/adjacent.hpp>
#include <sort_algorithm/compare.hpp>
#include <sort_algorithm/yrange5.hpp>
#include <gui.hpp>


// 2値座標系式から1値座標系式に変えながら和
// 指定した範囲の配列の和を返す
int yrange5::array_sum(return_type const& array_, int const x, int const y, int const height, int const width) const
{
	int sum = 0;
	for (int i = 0; i<height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			auto const& target_point = array_[y + i][x + j];
			if (target_point.x == std::numeric_limits<int>::max() && target_point.y == std::numeric_limits<int>::max()) return -1;
			auto const point = target_point.y * width + target_point.x;
			sum += (point > 1000) ? 777 : point;
		}
	}
	return sum;
}

/*縦列単位で入れ替え*/
void yrange5::column_replacement(answer_type_y & answer)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;
	uint_fast64_t good_val;
	std::vector<std::vector<point_type> > good_matrix(sepy, std::vector<point_type>(sepx));
	good_matrix = answer.points;
	good_val = form_evaluate(data_,comp_, good_matrix);

	for (int i = 0; i < sepx; ++i){
		for (int j = 0; j < sepy; ++j){
			answer.points[j].insert(answer.points[j].begin(), answer.points[j][sepx - 1]);
			answer.points[j].pop_back();
		}
		auto const temp_score = form_evaluate(data_,comp_, answer.points);
		if (good_val>temp_score){
			good_val = temp_score;
			good_matrix = answer.points;
		}
	}
	answer.points = good_matrix;
	answer.score = good_val;
}

/*横列単位で入れ替え*/
void yrange5::row_replacement(answer_type_y& answer)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;
	uint_fast64_t good_val;
	std::vector<std::vector<point_type> > good_matrix(sepy, std::vector<point_type>(sepx));
	std::vector<point_type> temp_vec;
	good_matrix = answer.points;
	good_val = form_evaluate(data_,comp_, answer.points);
	for (int i = 0; i < sepy; ++i){
		answer.points.insert(answer.points.begin(), answer.points[sepy - 1]);
		answer.points.pop_back();
		auto const temp_score = form_evaluate(data_,comp_, answer.points);
		if (good_val>temp_score){
			good_val = temp_score;
			good_matrix = answer.points;
		}
	}
	answer.points = good_matrix;
	answer.score = good_val;
}

//cv::matの塊にする
cv::Mat yrange5::combine_image(answer_type_y const& answer)
{
	int const one_picx = data_.size.first / data_.split_num.first;
	int const one_picy = data_.size.second / data_.split_num.second;

	splitter sp;//どこからか持ってきてたsplitter
	split_image_type splitted = sp.split_image(data_);
	cv::Mat comb_pic(cv::Size(data_.size.first, data_.size.second), CV_8UC3);
	for (int i = 0; i < data_.split_num.second; ++i){
		for (int j = 0; j < data_.split_num.first; ++j){
			cv::Rect roi_rect(j*one_picx, i*one_picy, one_picx, one_picy);
			cv::Mat roi_mat(comb_pic, roi_rect);
			splitted[answer.points[i][j].y][answer.points[i][j].x].copyTo(roi_mat);
		}
	}
	return std::move(comb_pic.clone());
}


yrange5::yrange5(question_raw_data const& data, compared_type const& comp)
    : data_(data), comp_(comp), adjacent_data_(select_minimum(comp))
{
}

std::vector<std::vector<std::vector<point_type>>> yrange5::operator() ()
{
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;

	auto const exists = [height, width](point_type const& p)
	{
		return 0 <= p.x && p.x < width && 0 <= p.y && p.y < height;
	};


	std::vector<kind_rgb>kind_rgb_vector(width*height);
	std::vector<answer_type_y> to_iddfs;//kid_rgbで選考したもの入れとく
	std::vector<answer_type_y> answer;
	splitter sp;

	//すべてのピースから並べ始めるためのループ
	for (int c_y = 0; c_y < height; ++c_y) for (int c_x = 0; c_x < width; ++c_x)
	{
		std::vector<std::vector<point_type>> sorted_matrix(
			height * 2 - 1,
			std::vector<point_type>(
			width * 2 - 1,
			{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() }
			)
			);

		sorted_matrix[height - 1][width - 1] = point_type{ c_x, c_y };

		//上に見ていく
		for (int i = 0; i < height - 1; ++i)
		{
			auto const& adjacent = sorted_matrix[height - 1 - i][width - 1];
			sorted_matrix[height - 2 - i][width - 1] = adjacent_data_[adjacent.y][adjacent.x].up;
			if (sorted_matrix[height - 2 - i][width - 1].y < 0 || sorted_matrix[height - 2 - i][width - 1].y >= height) break;
		}
		//下に見ていく
		for (int i = 0; i < height - 1; ++i)
		{
			auto const& adjacent = sorted_matrix[height - 1 + i][width - 1];
			sorted_matrix[height + i][width - 1] = adjacent_data_[adjacent.y][adjacent.x].down;
			if (sorted_matrix[height + i][width - 1].y < 0 || sorted_matrix[height + i][width - 1].y >= height) break;
		}
		//右に見ていく
		for (int i = 0; i < width - 1; ++i)
		{
			auto const& adjacent = sorted_matrix[height - 1][width - 1 + i];
			sorted_matrix[height - 1][width + i] = adjacent_data_[adjacent.y][adjacent.x].right;
			if (sorted_matrix[height - 1][width + i].x < 0 || sorted_matrix[height - 1][width + i].x >= width) break;
		}
		//左に見ていく
		for (int i = 0; i < width - 1; ++i)
		{
			auto const& adjacent = sorted_matrix[height - 1][width - 1 - i];
			sorted_matrix[height - 1][width - 2 - i] = adjacent_data_[adjacent.y][adjacent.x].left;
			if (sorted_matrix[height - 1][width - 2 - i].x < 0 || sorted_matrix[height - 1][width - 2 - i].x >= width) break;
		}

		//中心を除き上に向かってループ，右に見ていく
		for (int i = 0; i < height - 1; ++i) for (int j = 0; j < width - 1; ++j)
		{
			auto const& center = sorted_matrix[height - i - 1][width + j - 1];
			auto const& upper = sorted_matrix[height - i - 2][width + j - 1];
			auto const& right = sorted_matrix[height - i - 1][width + j];

			if (exists(center) && exists(upper) && exists(right))
				sorted_matrix[height - i - 2][width + j] = ur_choose(comp_, upper, center, right);
			else
				break;
		}
		//中心を除き上に向かってループ，左に見ていく
		for (int i = 0; i < height - 1; ++i) for (int j = 0; j < width - 1; ++j)
		{
			auto const& center = sorted_matrix[height - i - 1][width - j - 1];
			auto const& upper = sorted_matrix[height - i - 2][width - j - 1];
			auto const& left = sorted_matrix[height - i - 1][width - j - 2];

			if (exists(center) && exists(upper) && exists(left))
				sorted_matrix[height - i - 2][width - j - 2] = ul_choose(comp_, upper, left, center);
			else
				break;
		}
		//中心を除き下に向かってループ，右に見ていく
		for (int i = 0; i < height - 1; ++i) for (int j = 0; j < width - 1; ++j)
		{
			auto const& center = sorted_matrix[height + i - 1][width + j - 1];
			auto const& lower = sorted_matrix[height + i][width + j - 1];
			auto const& right = sorted_matrix[height + i - 1][width + j];

			if (exists(center) && exists(lower) && exists(right))
				sorted_matrix[height + i][width + j] = dr_choose(comp_, center, right, lower);
			else
				break;
		}
		//中心を除き下に向かってループ，左に見ていく
		for (int i = 0; i < height - 1; ++i) for (int j = 0; j < width - 1; ++j)
		{
			auto const& center = sorted_matrix[height + i - 1][width - j - 1];
			auto const& lower = sorted_matrix[height + i][width - j - 1];
			auto const& left = sorted_matrix[height + i - 1][width - j - 2];

			if (exists(center) && exists(lower) && exists(left))
				sorted_matrix[height + i][width - j - 2] = dl_choose(comp_, left, center, lower);
			else
				break;
		}

		//kind_rgb選考
		for (int i = 0; i < height; i++)	for (int j = 0; j < width; j++){
			kind_rgb_vector[j * height + i].x = j;
			kind_rgb_vector[j * height + i].y = i;
			kind_rgb_vector[j * height + i].kind = get_kind_num(data_, sorted_matrix, j, i);
			kind_rgb_vector[j * height + i].score = (width*height - kind_rgb_vector[j * height + i].kind + 1) * range_evaluate(data_, comp_, sorted_matrix, j, i);
		}

		std::sort(kind_rgb_vector.begin(), kind_rgb_vector.end());

		for (int k = 0; k < kind_rgb_vector.size() / 4; k++){
			std::vector<std::vector <point_type>> ans_temp2;
			for (int i = 0; i < height; ++i){
				std::vector<point_type>ans_temp1;
				for (int j = 0; j < width; ++j){
					ans_temp1.push_back(sorted_matrix[kind_rgb_vector[k].y + i][kind_rgb_vector[k].x + j]);
				}
				ans_temp2.push_back(ans_temp1);
			}
			//TODO:ここどうにかしよう
			answer_type_y temp;
			temp.points = ans_temp2;
			to_iddfs.push_back(std::move(temp));
		}
	}

	//yrange5メインループ
	for (auto& matrix : to_iddfs){
		//重複して入っているもののうち，悪い方を{width,height}に置き換え,今使われていないものを返す
		std::vector<point_type> usable =std::move(duplicate_delete(comp_, matrix.points));
		//gui::combine_show_image(data_, comp_, matrix.points);
		//I'ts show time!
		for (int i = 0; i < height; ++i)for (int j = 0; j < width; ++j){
			std::vector<std::vector<point_type>> sorted_matrix(height, std::vector<point_type>(width, point_type{ width, height }));
			//全部並べ切れたら
			if (!try_matrix_copy(sorted_matrix, matrix.points, point_type{ j, i }))
			{
				//抜けてるところを並べていく
				point_type const invalid_val = { width, height };
				int const u_height = (height % 2 == 0) ? height / 2 + 1 : height / 2;
				int const r_width  = width / 2;
				int const d_height = height / 2;
				int const l_width  = (width % 2 == 0) ? height / 2 + 1 : width / 2;

				//上に見ていく
				for (int i = 0; i < height - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[height - 1 - i][width / 2];
					auto      & target   = sorted_matrix[height - 2 - i][width / 2];
					if (target == invalid_val && adjacent != invalid_val) target = u_choose(comp_, adjacent, usable);
				}
				//下に見ていく
				for (int i = 0; i < height - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[i][width / 2];
					auto      & target   = sorted_matrix[i + 1][width / 2];
					if (target == invalid_val && adjacent != invalid_val) target = d_choose(comp_, adjacent, usable);
				}
				//右に見ていく
				for (int i = 0; i < width - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[height / 2][i];
					auto      & target   = sorted_matrix[height / 2][i + 1];
					if (target == invalid_val && adjacent != invalid_val) target = r_choose(comp_, adjacent, usable);
				}
				//左に見ていく
				for (int i = 0; i < width - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[height / 2][width - 1 - i];
					auto      & target   = sorted_matrix[height / 2][width - 2 - i];
					if (target == invalid_val && adjacent != invalid_val) target = l_choose(comp_, adjacent, usable);
				}

				//中心を除き上に向かってループ，右に見ていく
				for (int i = 0; i < u_height - 1; ++i) for (int j = 0; j < r_width - 1; ++j)
				{
					auto      & target = sorted_matrix[height / 2 - 1 - i][width / 2 + 1 + j];
					auto const& center = sorted_matrix[height / 2 - i    ][width / 2 + j    ];
					auto const& upper  = sorted_matrix[height / 2 - i - 1][width / 2 + j    ];
					auto const& right  = sorted_matrix[height / 2 - i    ][width / 2 + j + 1];

					if (target == invalid_val && (exists(center) && exists(upper) && exists(right)))
						target = ur_choose(comp_, upper, center, right,usable);
				}
				//中心を除き上に向かってループ，左に見ていく
				for (int i = 0; i < u_height - 1; ++i) for (int j = 0; j < l_width - 1; ++j)
				{
					auto      & target = sorted_matrix[height / 2 - i - 1][width / 2 - j - 1];
					auto const& center = sorted_matrix[height / 2 - i    ][width / 2 - j];
					auto const& upper  = sorted_matrix[height / 2 - i - 1][width / 2 - j];
					auto const& left   = sorted_matrix[height / 2 - i    ][width / 2 - j - 1];

					if (target == invalid_val && (exists(center) && exists(upper) && exists(left)))
						target = ul_choose(comp_, upper, left, center,usable);
				}
				//中心を除き下に向かってループ，右に見ていく
				for (int i = 0; i < d_height - 1; ++i) for (int j = 0; j < r_width - 1; ++j)
				{
					auto	  & target = sorted_matrix[height / 2 + i + 1][width / 2 + j + 1];
					auto const& center = sorted_matrix[height / 2 + i    ][width / 2 + j];
					auto const& lower  = sorted_matrix[height / 2 + i + 1][width / 2 + j];
					auto const& right  = sorted_matrix[height / 2 + i    ][width / 2 + j + 1];

					if (target == invalid_val && (exists(center) && exists(lower) && exists(right)))
						target = dr_choose(comp_, center, right, lower,usable);
				}
				//中心を除き下に向かってループ，左に見ていく
				for (int i = 0; i < d_height - 1; ++i) for (int j = 0; j < l_width - 1; ++j)
				{
					auto      & target = sorted_matrix[height / 2 + i + 1][width / 2 - j - 1];
					auto const& center = sorted_matrix[height / 2 + i    ][width / 2 - j];
					auto const& lower  = sorted_matrix[height / 2 + i + 1][width / 2 - j];
					auto const& left   = sorted_matrix[height / 2 + i    ][width / 2 - j - 1];

					if (target == invalid_val && (exists(center) && exists(lower) && exists(left)))
						target = dl_choose(comp_, left, center, lower,usable);
				}
				//gui::combine_show_image(data_, comp_, sorted_matrix);
				if (array_sum(sorted_matrix, 0, 0, height, width) == ((width*height - 1)*(width*height) / 2) && get_kind_num(data_, sorted_matrix, 0, 0) == width*height)
				{
					//TODO:ここどうにかしよう
					answer_type_y temp;
					temp.points = sorted_matrix;
					answer.push_back(std::move(temp));
				}
			}
		}
	}

	//現段階で重複しているものは1つに絞る
	// unique()を使う準備としてソートが必要
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.points < b.points; });
	// unique()をしただけでは後ろにゴミが残るので、eraseで削除する
	answer.erase(std::unique(answer.begin(), answer.end()), answer.end());

	//縦入れ替え，横入れ替え
	for (auto &matrix : answer){
		row_replacement(matrix);
		column_replacement(matrix);
		row_replacement(matrix);
	}

	//もっかいやっとく
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.points < b.points; });
	answer.erase(std::unique(answer.begin(), answer.end()), answer.end());

	//無駄に多く返してもしょうがないので枝抜き
	size_t yrange5_ans = answer.size();
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.score < b.score; });
	if (answer.size() >= 8) answer.resize(8);

	//一枚のcv::Matにする
	for (auto& one_answer : answer)
	{
		one_answer.mat_image = std::move(combine_image(one_answer));
	}

#ifdef _DEBUG
	std::cout << "There are " << yrange5_ans << " solutions by yrange5." << std::endl;
	for (auto const& one_answer : answer)
	{
		for (int i = 0; i < one_answer.points.size(); ++i)
		{
			for (int j = 0; j < one_answer.points.at(0).size(); ++j)
			{
				auto const& data = one_answer.points[i][j];
				std::cout << boost::format("(%2d,%2d) ") % data.x % data.y;
			}
			std::cout << "\n";
		}
		std::cout << "score = " << one_answer.score << std::endl;
	}
	gui::show_image(data_, comp_, answer);
#endif
	//TODO:ここどうにかしよう
	std::vector<return_type> return_vec;
	for (auto const& one_answer : answer)
	{
		return_vec.push_back(std::move(one_answer.points));
	}
	return return_vec;
}

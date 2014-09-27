﻿#ifdef _DEBUG
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
    for(int i=0; i<height; ++i)
    {
        for(int j=0; j<width; ++j)
        {
            auto const& target_point = array_[y + i][x + j];
			if (target_point.x == std::numeric_limits<int>::max() && target_point.y == std::numeric_limits<int>::max()) return -1;
			else
            {
                auto const point = target_point.y * width + target_point.x;
                sum += (point > 1000) ? 777 : point;
            }
        }
    }
    return sum;
}

/*縦列単位で入れ替え*/
void yrange5::column_replacement(return_type & matrix)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;
	uint_fast64_t good_val;
	std::vector<std::vector<point_type> > good_matrix(sepy, std::vector<point_type>(sepx));

	good_matrix = matrix;
	good_val = form_evaluate(comp_,good_matrix);

	for (int i = 0; i < sepx; ++i){
		for (int j = 0; j < sepy; ++j){
			matrix[j].insert(matrix[j].begin(), matrix[j][sepx - 1]);
			matrix[j].pop_back();
		}
		auto const& temp_score = form_evaluate(comp_, matrix);
		if (good_val>temp_score){
			good_val = temp_score;
			good_matrix = matrix;
		}
	}
	matrix = good_matrix;
}

/*横列単位で入れ替え*/
void yrange5::row_replacement(return_type& matrix)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;
	uint_fast64_t good_val;
	std::vector<std::vector<point_type> > good_matrix(sepy, std::vector<point_type>(sepx));
	std::vector<point_type> temp_vec;

	good_matrix = matrix;
	good_val = form_evaluate(comp_,matrix);
	for (int i = 0; i < sepy; ++i){
		matrix.insert(matrix.begin(), matrix[sepy - 1]);
		matrix.pop_back();
		auto const& temp_score = form_evaluate(comp_, matrix);
		if (good_val>temp_score){
			good_val = temp_score;
			good_matrix = matrix;
		}
	}
	matrix = good_matrix;

}

//cv::matの塊にする
std::vector<cv::Mat> yrange5::combine_image(std::vector<std::vector<std::vector<point_type>>>const & matrix)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;
	int const picx = data_.size.first;
	int const picy = data_.size.second;
	int const one_picx = picx / sepx;
	int const one_picy = picy / sepy;

	std::vector<cv::Mat> answer;
	splitter sp;//どこからか持ってきてたsplitter
	split_image_type splitted = sp.split_image(data_);
	for (auto arr : matrix){
		cv::Mat comb_pic(cv::Size(picx, picy), CV_8UC3);
		for (int i = 0; i < data_.split_num.second; ++i){
			for (int j = 0; j < data_.split_num.first; ++j){
				cv::Rect roi_rect(j*one_picx, i*one_picy, one_picx, one_picy);
				cv::Mat roi_mat(comb_pic, roi_rect);
				splitted[arr[i][j].y][arr[i][j].x].copyTo(roi_mat);
			}
		}
		answer.push_back(comb_pic.clone());
	}
	return answer;
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
	std::vector<std::vector<std::vector<point_type> > > to_iddfs;//kid_rgbで選考したもの入れとく

	splitter sp;
	answer_type_y answer;

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
			to_iddfs.push_back(ans_temp2);
		}
	}

	//yrange5メインループ
	for (auto& matrix : to_iddfs){
		//重複して入っているもののうち，悪い方を{width,height}に置き換え,今使われていないものを返す
		std::vector<point_type> usable =std::move(duplicate_delete(comp_, matrix));
		//gui::combine_show_image(data_, comp_, matrix);
		//I'ts show time!
		for (int i = 0; i < height; ++i)for (int j = 0; j < width; ++j){
			std::vector<std::vector<point_type>> sorted_matrix(height, std::vector<point_type>(width, point_type{ width, height }));
			//全部並べ切れたら
			if (!try_matrix_copy(sorted_matrix, matrix, point_type{ j, i }))
			{
				//抜けてるところを並べていく
				point_type const invalid_val = { width, height };
				int const u_height = (height % 2 == 0) ? height / 2 + 1 : height / 2;
				int const r_width = width / 2;
				int const d_height = height / 2;
				int const l_width = (width % 2 == 0) ? height / 2 + 1 : width / 2;

				//上に見ていく
				for (int i = 0; i < height - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[height - 1 - i][width / 2];
					auto      & target = sorted_matrix[height - 2 - i][width / 2];
					if (target == invalid_val && adjacent != invalid_val) target = adjacent_data_[adjacent.y][adjacent.x].up;
				}
				//下に見ていく
				for (int i = 0; i < height - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[i][width / 2];
					auto      & target = sorted_matrix[i + 1][width / 2];
					if (target == invalid_val && adjacent != invalid_val) target = adjacent_data_[adjacent.y][adjacent.x].down;
				}
				//右に見ていく
				for (int i = 0; i < width - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[height / 2][i];
					auto      & target = sorted_matrix[height / 2][i + 1];
					if (target == invalid_val && adjacent != invalid_val) target = adjacent_data_[adjacent.y][adjacent.x].right;
				}
				//左に見ていく
				for (int i = 0; i < width - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[height / 2][width - 1 - i];
					auto      & target = sorted_matrix[height / 2][width - 2 - i];
					if (target == invalid_val && adjacent != invalid_val) target = adjacent_data_[adjacent.y][adjacent.x].left;
				}

				//中心を除き上に向かってループ，右に見ていく
				for (int i = 0; i < u_height - 1; ++i) for (int j = 0; j < r_width - 1; ++j)
				{
					auto      & target = sorted_matrix[height / 2 - 1 - i][width / 2 + 1 + j];
					auto const& center = sorted_matrix[height / 2 - i    ][width / 2 + j    ];
					auto const& upper  = sorted_matrix[height / 2 - i - 1][width / 2 + j    ];
					auto const& right  = sorted_matrix[height / 2 - i    ][width / 2 + j + 1];

					if (target == invalid_val && (exists(center) && exists(upper) && exists(right)))
						target = ur_choose(comp_, upper, center, right);
				}
				//中心を除き上に向かってループ，左に見ていく
				for (int i = 0; i < u_height - 1; ++i) for (int j = 0; j < l_width - 1; ++j)
				{
					auto      & target = sorted_matrix[height / 2 - i - 1][width / 2 - j - 1];
					auto const& center = sorted_matrix[height / 2 - i][width / 2 - j];
					auto const& upper = sorted_matrix[height / 2 - i - 1][width / 2 - j];
					auto const& left = sorted_matrix[height / 2 - i][width / 2 - j - 1];

					if (target == invalid_val && (exists(center) && exists(upper) && exists(left)))
						target = ul_choose(comp_, upper, left, center);
				}
				//中心を除き下に向かってループ，右に見ていく
				for (int i = 0; i < d_height - 1; ++i) for (int j = 0; j < r_width - 1; ++j)
				{
					auto	  & target = sorted_matrix[height / 2 + i + 1][width / 2 + j + 1];
					auto const& center = sorted_matrix[height / 2 + i][width / 2 + j];
					auto const& lower = sorted_matrix[height / 2 + i + 1][width / 2 + j];
					auto const& right = sorted_matrix[height / 2 + i][width / 2 + j + 1];

					if (target == invalid_val && (exists(center) && exists(lower) && exists(right)))
						target = dr_choose(comp_, center, right, lower);
				}
				//中心を除き下に向かってループ，左に見ていく
				for (int i = 0; i < d_height - 1; ++i) for (int j = 0; j < l_width - 1; ++j)
				{
					auto      & target = sorted_matrix[height / 2 + i + 1][width / 2 - j - 1];
					auto const& center = sorted_matrix[height / 2 + i][width / 2 - j];
					auto const& lower = sorted_matrix[height / 2 + i + 1][width / 2 - j];
					auto const& left = sorted_matrix[height / 2 + i][width / 2 - j - 1];

					if (target == invalid_val && (exists(center) && exists(lower) && exists(left)))
						target = dl_choose(comp_, left, center, lower);
				}
				gui::combine_show_image(data_, comp_, sorted_matrix);
				if (array_sum(sorted_matrix, 0, 0, height, width) == ((width*height - 1)*(width*height) / 2) && get_kind_num(data_, sorted_matrix, 0, 0) == width*height)
				{
					answer.points.push_back(sorted_matrix);
				}
			}
		}
	}

	//現段階で重複しているものは1つに絞る
	// unique()を使う準備としてソートが必要
	std::sort(answer.points.begin(), answer.points.end());
	// unique()をしただけでは後ろにゴミが残るので、eraseで削除する
	answer.points.erase(std::unique(answer.points.begin(), answer.points.end()), answer.points.end());

	//縦入れ替え，横入れ替え
	for (auto &matrix : answer.points){
		row_replacement(matrix);
		column_replacement(matrix);
		row_replacement(matrix);
	}

	//もっかいやっとく
	std::sort(answer.points.begin(), answer.points.end());
	answer.points.erase(std::unique(answer.points.begin(), answer.points.end()), answer.points.end());

	//一枚のcv::Matにする
	answer.mat_image = combine_image(answer.points);

#ifdef _DEBUG
	std::cout << "There are " << answer.points.size() << " solutions by yrange5." << std::endl;
	for (auto const& one_answer : answer.points)
	{
		for (int i = 0; i < one_answer.size(); ++i)
		{
			for (int j = 0; j < one_answer.at(0).size(); ++j)
			{
				auto const& data = one_answer[i][j];
				std::cout << boost::format("(%2d,%2d) ") % data.x % data.y;
			}
			std::cout << "\n";
		}
		std::cout << "score = " << form_evaluate(comp_, one_answer) << std::endl;
	}
	gui::show_image(data_, comp_, answer);
#endif
	return to_iddfs;
}

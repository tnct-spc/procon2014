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
#include <sort_algorithm/yrange2.hpp>
#include <gui.hpp>

// 2値座標系式から1値座標系式に変えながら和
// 指定した範囲の配列の和を返す
int yrange2::array_sum(return_type const& array_, int const x, int const y, int const height, int const width) const
{
    int sum = 0;
    for(int i=0; i<height; ++i)
    {
        for(int j=0; j<width; ++j)
        {
            auto const& target_point = array_[y + i][x + j];
            if(target_point.x != std::numeric_limits<int>::max() && target_point.y != std::numeric_limits<int>::max())
            {
                auto const point = target_point.y * width + target_point.x;
                sum += (point > 1000) ? 777 : point;
            }
        }
    }
    return sum;
}

/*縦入れ替え*/
void yrange2::column_replacement(return_type& matrix)
{
	const int sepx = data_.split_num.first;
	const int sepy = data_.split_num.second;
	uint64_t good_val;
	std::vector<std::vector<point_type> > good_matrix(sepx, (sepx, std::vector<point_type>(sepy)));
	std::vector<point_type> temp_vec;

	good_matrix = matrix;
	good_val = form_evaluate(comp_,matrix);

	for (int i = 0; i < sepx; ++i){
		for (int j = 0; j < sepy; ++j){
			matrix[j].push_back(matrix[j][sepx - 1]);
			matrix[j].pop_back();
		}

		if (good_val>form_evaluate(comp_,matrix)){
			good_val = form_evaluate(comp_,matrix);
			good_matrix = matrix;
		}
	}
	matrix = good_matrix;
}

/*横入れ替え*/
void yrange2::row_replacement(return_type& matrix)
{
	const int sepx = data_.split_num.first;
	const int sepy = data_.split_num.second;
	uint64_t good_val;
	std::vector<std::vector<point_type> > good_matrix(sepx, (sepx, std::vector<point_type>(sepy)));
	std::vector<point_type> temp_vec;

	good_matrix = matrix;
	good_val = form_evaluate(comp_,matrix);

	for (int i = 0; i < sepy; ++i){
		matrix.push_back(matrix[sepy - 1]);
		matrix.pop_back();
	}
	if (good_val>form_evaluate(comp_,matrix)){
		good_val = form_evaluate(comp_,matrix);
		good_matrix = matrix;
	}
	matrix = good_matrix;
}

//cv::matの塊にする
std::vector<cv::Mat> yrange2::combine_image(std::vector<std::vector<std::vector<point_type>>>const & matrix)
{
	std::vector<cv::Mat> answer;
	cv::Mat comb_pic(cv::Size(data_.size.first, data_.size.second), CV_8UC3);
	cv::Rect roi_rect;
	roi_rect.height = (data_.size.second / data_.split_num.second);// picy/sepy
	roi_rect.width = (data_.size.first / data_.split_num.first);// picx/sepx
	splitter sp;//どこからか持ってきてたsplitter
	split_image_type splitted = sp.split_image(data_);
	for (auto arr : matrix){
		for (int i = 0; i < data_.split_num.second; i++){
			for (int j = 0; j < data_.split_num.first; j++){
				cv::Mat roi(comb_pic, roi_rect);
				splitted[arr[i][j].y][arr[i][j].x].copyTo(roi);
				roi_rect.x += (data_.size.first / data_.split_num.first);
			}
			roi_rect.x = 0;
			roi_rect.y += (data_.size.second / data_.split_num.second);
		}
		answer.push_back(comb_pic);
	}
	return answer;
}


yrange2::yrange2(question_raw_data const& data, compared_type const& comp)
    : data_(data), comp_(comp), adjacent_data_(select_minimum(comp))
{
}

std::vector<std::vector<std::vector<point_type>>> yrange2::operator() ()
{
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;

	auto const exists = [height, width](point_type const& p)
	{
		return 0 <= p.x && p.x < width && 0 <= p.y && p.y < height;
	};

	std::vector<std::vector<point_type>> sorted_matrix(
		height * 2 - 1,
		std::vector<point_type>(
		width * 2 - 1,
		{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() }
		)
		);

	answer_type_y answer;
	cr_set cr;
	splitter sp;
	cr = sp.make_column_row_set(data_);


	//すべてのピースから並べ始めるためのループ
	for (int c_y = 0; c_y < height; ++c_y) for (int c_x = 0; c_x < width; ++c_x)
	{
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
			auto const& left = sorted_matrix[height + i + j][width - j - 2];

			if (exists(center) && exists(lower) && exists(left))
				sorted_matrix[height + i][width + j] = dl_choose(comp_, left, center, lower);
			else
				break;
		}

		for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x)
		{
			if (array_sum(sorted_matrix, x, y, height, width) == ((width*height - 1)*(width*height) / 2) && get_kind_num(data_, sorted_matrix, x, y) == width*height)
			{
				std::vector<std::vector<point_type>> one_answer(height, std::vector<point_type>(width));
				for (int i = 0; i < height; i++) for (int j = 0; j < width; j++)
				{
					one_answer[i][j] = sorted_matrix[y + i][x + j];
				}
				answer.point_type.push_back(std::move(one_answer));
			}
		}
	}

	//現段階で重複しているものは1つに絞る
	// unique()を使う準備としてソートが必要
	std::sort(answer.point_type.begin(), answer.point_type.end());
	// unique()をしただけでは後ろにゴミが残るので、eraseで削除する
	answer.point_type.erase(std::unique(answer.point_type.begin(), answer.point_type.end()), answer.point_type.end());

	
	//#########################################################yrange2.5#########################################################//

	//縦入れ替え，横入れ替え
	for (auto matrix : answer.point_type){
		row_replacement(matrix);
		column_replacement(matrix);
	}

	//もっかいやっとく
	std::sort(answer.point_type.begin(), answer.point_type.end());
	answer.point_type.erase(std::unique(answer.point_type.begin(), answer.point_type.end()), answer.point_type.end());

	//一枚のcv::Matにする
	answer.cv_Mat = combine_image(answer.point_type);

//	std::cout << cv::arcLength(answer.cv_Mat[0], true) << std::endl;
//	std::cout << cv::arcLength(answer.cv_Mat[0], false) << std::endl;


#ifdef _DEBUG
    std::cout << "There are " << answer.point_type.size() << " solutions" << std::endl;
	for (auto const& one_answer : answer.point_type)
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
	
	point_type a = { 0, 0 };
	point_type b = { 0, 1 };
	point_type c = { 0, 2 };

	//ur_choose(comp_, cr, CV_TM_SQDIFF, a, b, c);

    return answer.point_type;
}

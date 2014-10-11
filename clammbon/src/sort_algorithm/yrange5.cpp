#ifdef _DEBUG
#include <iostream>
#include <boost/format.hpp>
#endif

#include <vector>
#include <algorithm>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <omp.h>
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

/*縦横全パターンやろう*/
void yrange5::row_column_replacement(answer_type_y& answer)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;

	struct good_set
	{
		uint_fast64_t val;
		point_type point;
	};
	good_set good;

	std::vector<std::vector<point_type>>sort_matrix(answer.points.size() * 2, std::vector<point_type>(answer.points.at(0).size() * 2));
#pragma omp parallel for
	for (int i = 0; i < answer.points.size(); ++i)for (int j = 0; j < answer.points.at(0).size(); ++j)
	{
		sort_matrix[i][j] = answer.points[i][j];
		sort_matrix[sepy + i][j] = answer.points[i][j];
		sort_matrix[i][sepx + j] = answer.points[i][j];
		sort_matrix[sepy + i][sepx + j] = answer.points[i][j];
	}

	good = good_set{ range_evaluate_contours(data_, comp_, answer.points, 0, 0), { 0, 0 } };
	for (int i = 0; i < sepy; ++i)for (int j = 0; j < sepx; ++j)
	{
		auto const& temp = range_evaluate_contours(data_, comp_, sort_matrix, j, i);
		if (good.val > temp)good = { temp, { j, i } };
	}

	std::vector<std::vector<point_type>>temp_matrix(sepy, std::vector<point_type>(sepx));
#pragma omp parallel for
	for (int i = 0; i < sepy; ++i)for (int j = 0; j < sepx; ++j)
	{
		temp_matrix[i][j] = sort_matrix[i][j];
	}

	answer = { std::move(temp_matrix), good.val, cv::Mat() };
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

std::vector<answer_type_y> yrange5::operator() (std::vector<std::vector<std::vector<point_type>>> const& mother_matrix)
{
	
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	constexpr int paramerter = 50;
	constexpr int yrange5_show_ans = 8;

	auto const exists = [height, width](point_type const& p)
	{
		return 0 <= p.x && p.x < width && 0 <= p.y && p.y < height;
	};

	std::vector<kind_rgb>kind_rgb_vector(width*height);
	std::vector<answer_type_y> to_y5;
	to_y5.reserve(width*height * 4);
	std::vector<answer_type_y> answer;
	answer.reserve(height*width * 2);
	splitter sp;

	omp_init_lock(&ol);

#pragma omp parallel for
	for (int c = 0; c < mother_matrix.size(); ++c)
	{
		//kind_rgb選考
		for (int i = 0; i < height; i++)	for (int j = 0; j < width; j++){
			kind_rgb_vector[j * height + i].x = j;
			kind_rgb_vector[j * height + i].y = i;
			kind_rgb_vector[j * height + i].kind = get_kind_num(data_, mother_matrix.at(c), j, i);
			kind_rgb_vector[j * height + i].score = (width*height - kind_rgb_vector[j * height + i].kind + 1) * range_evaluate(data_, comp_, mother_matrix.at(c), j, i);
		}

		std::sort(kind_rgb_vector.begin(), kind_rgb_vector.end());

		for (int k = 0; k < kind_rgb_vector.size() / paramerter; k++){
			std::vector<std::vector <point_type>> ans_temp(height, std::vector<point_type>(width));
			for (int i = 0; i < height; ++i) for (int j = 0; j < width; ++j)
			{
				ans_temp[i][j] = mother_matrix.at(c)[kind_rgb_vector[k].y + i][kind_rgb_vector[k].x + j];
			}
			omp_set_lock(&ol);
			to_y5.push_back(answer_type_y{ std::move(ans_temp), 0, cv::Mat() });
			omp_unset_lock(&ol);
		}
	}
	
	//yrange5メインループ
#pragma omp parallel for
	for (int c = 0; c < to_y5.size(); ++c)
	{
		//重複して入っているもののうち，悪い方を{width,height}に置き換え,今使われていないものを返す
		std::vector<point_type> usable = std::move(duplicate_delete(comp_, to_y5.at(c).points));
		//It's show time!
		for (int i = 0; i < height; ++i)for (int j = 0; j < width; ++j)
		{
			std::vector<std::vector<point_type>> sorted_matrix(height, std::vector<point_type>(width, point_type{ width, height }));
			//全部並べ切れたら
			if (!try_matrix_copy(sorted_matrix, to_y5.at(c).points, point_type{ j, i }))
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
					if (target == invalid_val && adjacent != invalid_val) target = u_choose(comp_, adjacent, usable);
				}
				//下に見ていく
				for (int i = 0; i < height - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[i][width / 2];
					auto      & target = sorted_matrix[i + 1][width / 2];
					if (target == invalid_val && adjacent != invalid_val) target = d_choose(comp_, adjacent, usable);
				}
				//右に見ていく
				for (int i = 0; i < width - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[height / 2][i];
					auto      & target = sorted_matrix[height / 2][i + 1];
					if (target == invalid_val && adjacent != invalid_val) target = r_choose(comp_, adjacent, usable);
				}
				//左に見ていく
				for (int i = 0; i < width - 1; ++i)
				{
					auto const& adjacent = sorted_matrix[height / 2][width - 1 - i];
					auto      & target = sorted_matrix[height / 2][width - 2 - i];
					if (target == invalid_val && adjacent != invalid_val) target = l_choose(comp_, adjacent, usable);
				}

				//中心を除き上に向かってループ，右に見ていく
				for (int i = 0; i < u_height - 1; ++i) for (int j = 0; j < r_width - 1; ++j)
				{
					auto      & target = sorted_matrix[height / 2 - 1 - i][width / 2 + 1 + j];
					auto const& center = sorted_matrix[height / 2 - i][width / 2 + j];
					auto const& upper = sorted_matrix[height / 2 - i - 1][width / 2 + j];
					auto const& right = sorted_matrix[height / 2 - i][width / 2 + j + 1];

					if (target == invalid_val && (exists(center) && exists(upper) && exists(right)))
						target = ur_choose(comp_, upper, center, right, usable);
				}
				//中心を除き上に向かってループ，左に見ていく
				for (int i = 0; i < u_height - 1; ++i) for (int j = 0; j < l_width - 1; ++j)
				{
					auto      & target = sorted_matrix[height / 2 - i - 1][width / 2 - j - 1];
					auto const& center = sorted_matrix[height / 2 - i][width / 2 - j];
					auto const& upper = sorted_matrix[height / 2 - i - 1][width / 2 - j];
					auto const& left = sorted_matrix[height / 2 - i][width / 2 - j - 1];

					if (target == invalid_val && (exists(center) && exists(upper) && exists(left)))
						target = ul_choose(comp_, upper, left, center, usable);
				}
				//中心を除き下に向かってループ，右に見ていく
				for (int i = 0; i < d_height - 1; ++i) for (int j = 0; j < r_width - 1; ++j)
				{
					auto	  & target = sorted_matrix[height / 2 + i + 1][width / 2 + j + 1];
					auto const& center = sorted_matrix[height / 2 + i][width / 2 + j];
					auto const& lower = sorted_matrix[height / 2 + i + 1][width / 2 + j];
					auto const& right = sorted_matrix[height / 2 + i][width / 2 + j + 1];

					if (target == invalid_val && (exists(center) && exists(lower) && exists(right)))
						target = dr_choose(comp_, center, right, lower, usable);
				}
				//中心を除き下に向かってループ，左に見ていく
				for (int i = 0; i < d_height - 1; ++i) for (int j = 0; j < l_width - 1; ++j)
				{
					auto      & target = sorted_matrix[height / 2 + i + 1][width / 2 - j - 1];
					auto const& center = sorted_matrix[height / 2 + i][width / 2 - j];
					auto const& lower = sorted_matrix[height / 2 + i + 1][width / 2 - j];
					auto const& left = sorted_matrix[height / 2 + i][width / 2 - j - 1];

					if (target == invalid_val && (exists(center) && exists(lower) && exists(left)))
						target = dl_choose(comp_, left, center, lower, usable);
				}
				if (array_sum(sorted_matrix, 0, 0, height, width) == ((width*height - 1)*(width*height) / 2) && get_kind_num(data_, sorted_matrix, 0, 0) == width*height)
				{
					omp_set_lock(&ol);
					answer.push_back(answer_type_y{ sorted_matrix, form_evaluate(data_,comp_,sorted_matrix), cv::Mat() });
					omp_unset_lock(&ol);
				}
			}
		}
	}

	//現段階で重複しているものは1つに絞る
	// unique()を使う準備としてソートが必要
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.points < b.points; });
	// unique()をしただけでは後ろにゴミが残るので、eraseで削除する
	answer.erase(std::unique(answer.begin(), answer.end()), answer.end());

	//無駄に多く返してもしょうがないので枝抜き
	size_t yrange5_ans = answer.size();
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.score < b.score; });
	if (answer.size() >= yrange5_show_ans) answer.resize(yrange5_show_ans);

//#pragma omp parallel for
//	for (int c = 0; c < answer.size(); ++c)
//	{
//		row_column_replacement(answer.at(c));
//	}
	
	//スコアの良いものが最上位レイヤーに来るように
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.score > b.score; });

#ifdef _DEBUG
//	//一枚のcv::Matにする
//#pragma omp parallel for
//	for (int c = 0; c < answer.size();++c)
//	{
//		answer.at(c).mat_image = std::move(combine_image(answer.at(c)));
//	}

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
	//gui::show_image(data_, comp_, answer);
#endif
	return answer;
}

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

//cv::matの塊にする
cv::Mat yrange2::combine_image(std::vector<std::vector<point_type>> matrix)
{
	int const one_picx = data_.size.first / data_.split_num.first;
	int const one_picy = data_.size.second / data_.split_num.second;

	splitter sp;//どこからか持ってきてたsplitter
	split_image_type splitted = sp.split_image(data_);
	cv::Mat comb_pic(cv::Size(data_.size.first, data_.size.second), CV_8UC3);
	for (int i = 0; i < data_.split_num.second; ++i) for (int j = 0; j < data_.split_num.first; ++j){
		cv::Rect roi_rect(j*one_picx, i*one_picy, one_picx, one_picy);
		cv::Mat roi_mat(comb_pic, roi_rect);
		splitted[matrix[i][j].y][matrix[i][j].x].copyTo(roi_mat);
	}

	return std::move(comb_pic.clone());
}

yrange2::yrange2(question_raw_data const& data, compared_type const& comp)
    : data_(data), comp_(comp), adjacent_data_(select_minimum(comp))
{
}

std::vector<answer_type_y> yrange2::operator() ()
{
	auto const width = data_.split_num.first;
	auto const height = data_.split_num.second;
	int constexpr yrange2_show_ans = 8;

	auto const exists = [height, width](point_type const& p)
	{
		return 0 <= p.x && p.x < width && 0 <= p.y && p.y < height;
	};

	std::vector<std::vector<std::vector<point_type>>> sorted_matrix(
		width*height,
		std::vector<std::vector<point_type>>(
		height * 2 - 1,
		std::vector<point_type>(
		width * 2 - 1,
		{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() }
		)
		)
		);

	std::vector<answer_type_y> answer;
	answer.reserve(width*height * 2);
	splitter sp;


#pragma omp parallel for
	for (int c = 0; c < width*height; ++c)
	{
		sorted_matrix.at(c)[height - 1][width - 1] = point_type{ c%width, c / width };
		//上に見ていく
		for (int i = 0; i < height - 1; ++i)
		{
			auto const& adjacent = sorted_matrix.at(c)[height - 1 - i][width - 1];
			sorted_matrix.at(c)[height - 2 - i][width - 1] = adjacent_data_[adjacent.y][adjacent.x].up;
			if (sorted_matrix.at(c)[height - 2 - i][width - 1].y < 0 || sorted_matrix.at(c)[height - 2 - i][width - 1].y >= height) break;
		}
		//下に見ていく
		for (int i = 0; i < height - 1; ++i)
		{
			auto const& adjacent = sorted_matrix.at(c)[height - 1 + i][width - 1];
			sorted_matrix.at(c)[height + i][width - 1] = adjacent_data_[adjacent.y][adjacent.x].down;
			if (sorted_matrix.at(c)[height + i][width - 1].y < 0 || sorted_matrix.at(c)[height + i][width - 1].y >= height) break;
		}
		//右に見ていく
		for (int i = 0; i < width - 1; ++i)
		{
			auto const& adjacent = sorted_matrix.at(c)[height - 1][width - 1 + i];
			sorted_matrix.at(c)[height - 1][width + i] = adjacent_data_[adjacent.y][adjacent.x].right;
			if (sorted_matrix.at(c)[height - 1][width + i].x < 0 || sorted_matrix.at(c)[height - 1][width + i].x >= width) break;
		}
		//左に見ていく
		for (int i = 0; i < width - 1; ++i)
		{
			auto const& adjacent = sorted_matrix.at(c)[height - 1][width - 1 - i];
			sorted_matrix.at(c)[height - 1][width - 2 - i] = adjacent_data_[adjacent.y][adjacent.x].left;
			if (sorted_matrix.at(c)[height - 1][width - 2 - i].x < 0 || sorted_matrix.at(c)[height - 1][width - 2 - i].x >= width) break;
		}

		//中心を除き上に向かってループ，右に見ていく
		for (int i = 0; i < height - 1; ++i) for (int j = 0; j < width - 1; ++j)
		{
			auto const& center = sorted_matrix.at(c)[height - i - 1][width + j - 1];
			auto const& upper = sorted_matrix.at(c)[height - i - 2][width + j - 1];
			auto const& right = sorted_matrix.at(c)[height - i - 1][width + j];

			if (exists(center) && exists(upper) && exists(right))
				sorted_matrix.at(c)[height - i - 2][width + j] = ur_choose(comp_, upper, center, right);
			else
				break;
		}
		//中心を除き上に向かってループ，左に見ていく
		for (int i = 0; i < height - 1; ++i) for (int j = 0; j < width - 1; ++j)
		{
			auto const& center = sorted_matrix.at(c)[height - i - 1][width - j - 1];
			auto const& upper = sorted_matrix.at(c)[height - i - 2][width - j - 1];
			auto const& left = sorted_matrix.at(c)[height - i - 1][width - j - 2];

			if (exists(center) && exists(upper) && exists(left))
				sorted_matrix.at(c)[height - i - 2][width - j - 2] = ul_choose(comp_, upper, left, center);
			else
				break;
		}
		//中心を除き下に向かってループ，右に見ていく
		for (int i = 0; i < height - 1; ++i) for (int j = 0; j < width - 1; ++j)
		{
			auto const& center = sorted_matrix.at(c)[height + i - 1][width + j - 1];
			auto const& lower = sorted_matrix.at(c)[height + i][width + j - 1];
			auto const& right = sorted_matrix.at(c)[height + i - 1][width + j];

			if (exists(center) && exists(lower) && exists(right))
				sorted_matrix.at(c)[height + i][width + j] = dr_choose(comp_, center, right, lower);
			else
				break;
		}
		//中心を除き下に向かってループ，左に見ていく
		for (int i = 0; i < height - 1; ++i) for (int j = 0; j < width - 1; ++j)
		{
			auto const& center = sorted_matrix.at(c)[height + i - 1][width - j - 1];
			auto const& lower = sorted_matrix.at(c)[height + i][width - j - 1];
			auto const& left = sorted_matrix.at(c)[height + i - 1][width - j - 2];

			if (exists(center) && exists(lower) && exists(left))
				sorted_matrix.at(c)[height + i][width - j - 2] = dl_choose(comp_, left, center, lower);
			else
				break;
		}
	}


	for (int c = 0; c < width*height; ++c)
	{
		for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x)
		{
			if (array_sum(sorted_matrix.at(c), x, y, height, width) == ((width*height - 1)*(width*height) / 2) && get_kind_num(data_, sorted_matrix.at(c), x, y) == width*height)
			{
				std::vector<std::vector<point_type>> one_answer(height, std::vector<point_type>(width));
				for (int i = 0; i < height; i++) for (int j = 0; j < width; j++)
				{
					one_answer[i][j] = sorted_matrix.at(c)[y + i][x + j];
				}
				answer.push_back(answer_type_y{one_answer,form_evaluate(data_,comp_,one_answer),combine_image(one_answer)});
			}
		}
	}

	//現段階で重複しているものは1つに絞る
	// unique()を使う準備としてソートが必要
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.points < b.points; });
	// unique()をしただけでは後ろにゴミが残るので、eraseで削除する
	answer.erase(std::unique(answer.begin(), answer.end()), answer.end());

	//無駄に多く返してもしょうがないので枝抜き
	int const yrange2_ans = answer.size();
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.score < b.score; });
	if (answer.size() >= yrange2_show_ans) answer.resize(yrange2_show_ans);

#ifdef _DEBUG
    std::cout << "There are " << yrange2_ans << " solutions" << std::endl;
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
	return answer;
}

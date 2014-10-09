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

/*縦列単位で入れ替え*/
void yrange2::column_replacement(answer_type_y & answer)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;
	uint_fast64_t good_val;
	std::vector<std::vector<point_type> > good_matrix(sepy, std::vector<point_type>(sepx));
	good_matrix = answer.points;
	good_val = form_evaluate(data_,comp_,good_matrix);

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
void yrange2::row_replacement(answer_type_y& answer)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;
	uint_fast64_t good_val;
	std::vector<std::vector<point_type> > good_matrix(sepy, std::vector<point_type>(sepx));
	std::vector<point_type> temp_vec;
	good_matrix = answer.points;
	good_val = form_evaluate(data_, comp_, answer.points);
	for (int i = 0; i < sepy; ++i){
		answer.points.insert(answer.points.begin(), answer.points[sepy - 1]);
		answer.points.pop_back();
		auto const temp_score = form_evaluate(data_, comp_, answer.points);
		if (good_val>temp_score){
			good_val = temp_score;
			good_matrix = answer.points;
		}
	}
	answer.points = good_matrix;
	answer.score = good_val;
}

//cv::matの塊にする
cv::Mat yrange2::combine_image(answer_type_y const& answer)
{
	int const one_picx = data_.size.first / data_.split_num.first;
	int const one_picy = data_.size.second / data_.split_num.second;

	splitter sp;//どこからか持ってきてたsplitter
	split_image_type splitted = sp.split_image(data_);
	cv::Mat comb_pic(cv::Size(data_.size.first, data_.size.second), CV_8UC3);
#pragma omp parallel
	for (int i = 0; i < data_.split_num.second; ++i){
		for (int j = 0; j < data_.split_num.first; ++j){
			cv::Rect roi_rect(j*one_picx, i*one_picy, one_picx, one_picy);
			cv::Mat roi_mat(comb_pic, roi_rect);
			splitted[answer.points[i][j].y][answer.points[i][j].x].copyTo(roi_mat);
		}
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

    std::vector<std::vector<std::vector<point_type>>>& sorted_matrix = sorted_matrix_;
    sorted_matrix.clear();
    sorted_matrix.resize(
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


	//すべてのピースから並べ始めるためのループ
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

	for (int c = 0; c < height*width; ++c)
	{
		for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x)
		{
			if (array_sum(sorted_matrix.at(c), x, y, height, width) == ((width*height - 1)*(width*height) / 2) && get_kind_num(data_, sorted_matrix.at(c), x, y) == width*height)
			{
				std::vector<std::vector<point_type>> one_answer(height, std::vector<point_type>(width, point_type{ -1, -1 }));
				for (int i = 0; i < height; i++) for (int j = 0; j < width; j++)
				{
					one_answer[i][j] = sorted_matrix.at(c)[y + i][x + j];
				}
				answer.push_back(answer_type_y{ one_answer, 0, cv::Mat() });
			}
		}
	}

	//現段階で重複しているものは1つに絞る
	// unique()を使う準備としてソートが必要
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.points < b.points; });
	// unique()をしただけでは後ろにゴミが残るので、eraseで削除する
	answer.erase(std::unique(answer.begin(), answer.end()), answer.end());

	//縦入れ替え，横入れ替え
#pragma omp parallel for
	for (int c = 0; c < answer.size(); ++c)
	{
		row_replacement(answer.at(c));
		column_replacement(answer.at(c));
	}

	//もっかいやっとく
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.points < b.points; });
	answer.erase(std::unique(answer.begin(), answer.end()), answer.end());

	//無駄に多く返してもしょうがないので枝抜き
	int const yrange2_ans = answer.size();
	std::sort(answer.begin(), answer.end(), [](answer_type_y a, answer_type_y b){return a.score < b.score; });
	if (answer.size() >= yrange2_show_ans) answer.resize(yrange2_show_ans);

	//一枚のcv::Matにする
#pragma omp parallel for
	for (int c = 0; c < answer.size(); ++c)
	{
		answer.at(c).mat_image = std::move(combine_image(answer.at(c)));
	}

#ifdef _DEBUG
    std::cout << "There are " << yrange2_ans << " solutions by yrange2." << std::endl;
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

std::vector<std::vector<std::vector<point_type>>> const& yrange2::sorted_matrix()
{
    return sorted_matrix_;
}

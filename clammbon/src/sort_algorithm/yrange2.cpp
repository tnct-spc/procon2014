﻿#ifdef _DEBUG
#include <iostream>
#include <boost/format.hpp>
#endif

#include <vector>
#include <algorithm>
#include <data_type.hpp>
#include <sort_algorithm/adjacent.hpp>
#include <sort_algorithm/compare.hpp>
#include <sort_algorithm/yrange2.hpp>

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

/*場を評価する関数*/
uint64_t yrange2::form_evaluate(std::vector<point_type>  matrix)
{
	const int sepx = data_.split_num.first;
	const int sepy = data_.split_num.second;
	unsigned _int64 s = 0;
	int i, j;

	for (i = 0; i < sepx*sepy; i++){
		if (i%sepx != 0)      s += comp_[matrix[i].y][matrix[i].x][matrix[i + 1   ].y][matrix[i + 1   ].x].right;
		if (i>sepx*(sepy - 1))s += comp_[matrix[i].y][matrix[i].x][matrix[i + sepx].y][matrix[i + sepx].x].down;
	}
	return s;
}

/*縦入れ替え*/
void yrange2::column_replacement(return_type& matrix)
{
	const int sepx = data_.split_num.first;
	const int sepy = data_.split_num.second;
	std::uint_fast64_t good_val;
	std::vector<point_type>good_matrix;

	for (auto& sort_matrix : matrix){
		good_matrix = sort_matrix;
		good_val = form_evaluate(good_matrix);
		for (int i = 0; i < sepx; i++){
			for (int j = 0; j < sepy; j++){
				sort_matrix.insert(sort_matrix.begin()+j*sepx, sort_matrix[(j + 1)*sepx]);
				sort_matrix.erase(sort_matrix.begin() + sepx);
			}
			if (good_val>form_evaluate(sort_matrix)){
				good_val = form_evaluate(sort_matrix);
				good_matrix = sort_matrix;
			}
		}
		sort_matrix = good_matrix;
	}
}

/*横入れ替え*/
void yrange2::horizontal_replacement(return_type& matrix)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;
	std::uint_fast64_t good_val;
	std::vector<point_type>good_matrix;
	int temp_val;

	matrix_good = matrix;
	good_val = form_evaluate(matrix);

	for (int i = 0; i < sepy; i++){
		for (int j = 0; j<sepx; j++){
			temp_val = matrix[j][sepy - 1];
			matrix[j].insert(matrix[j].begin(), temp_val);
			matrix[j].erase(matrix[j].end() - 1);
		}
		if (good_val>form_evaluate(matrix)){
			good_val = form_evaluate(matrix);
			matrix_good = matrix;
		}
	}
	matrix = matrix_good;
}

yrange2::yrange2(question_raw_data const& data, compared_type const& comp)
    : data_(data), comp_(comp), adjacent_data_(select_minimum(comp))
{
}

std::vector<std::vector<std::vector<point_type>>> yrange2::operator() ()
{
    auto const width  = data_.split_num.first;
    auto const height = data_.split_num.second;

    auto const exists = [height, width](point_type const& p)
    {
        return 0 <= p.x && p.x < width && 0 <= p.y && p.y < height;
    };

    std::vector<std::vector<point_type>> sorted_matrix(
        height*2-1,
        std::vector<point_type>(
            width*2-1,
            {std::numeric_limits<int>::max(), std::numeric_limits<int>::max()}
            )
        );

    std::vector<std::vector<std::vector<point_type>>> answer;

    //すべてのピースから並べ始めるためのループ
    for(int c_y=0; c_y<height; ++c_y) for(int c_x=0; c_x<width; ++c_x)
    {
        sorted_matrix[height-1][width-1] = point_type{c_x, c_y};

        //上に見ていく
        for(int i=0; i<height-1; ++i)
        {
            auto const& adjacent = sorted_matrix[height-1-i][width-1];
            sorted_matrix[height-2-i][width-1] = adjacent_data_[adjacent.y][adjacent.x].up;
            if(sorted_matrix[height-2-i][width-1].y < 0 || sorted_matrix[height-2-i][width-1].y >= height) break;
        }
        //下に見ていく
        for(int i=0; i<height-1; ++i)
        {
            auto const& adjacent = sorted_matrix[height-1+i][width-1];
            sorted_matrix[height+i][width-1] = adjacent_data_[adjacent.y][adjacent.x].down;
            if(sorted_matrix[height+i][width-1].y < 0 || sorted_matrix[height+i][width-1].y >= height) break;
        }
        //右に見ていく
        for(int i=0; i<width-1; ++i)
        {
            auto const& adjacent = sorted_matrix[height-1][width-1+i];
            sorted_matrix[height-1][width+i] = adjacent_data_[adjacent.y][adjacent.x].right;
            if(sorted_matrix[height-1][width+i].x < 0 || sorted_matrix[height-1][width+i].x >= width) break;
        }
        //左に見ていく
        for(int i=0; i<width-1; ++i)
        {
            auto const& adjacent = sorted_matrix[height-1][width-1-i];
            sorted_matrix[height-1][width-2-i] = adjacent_data_[adjacent.y][adjacent.x].left;
            if(sorted_matrix[height-1][width-2-i].x < 0 || sorted_matrix[height-1][width-2-i].x >= width) break;
        }

        //中心を除き上に向かってループ，右に見ていく
        for(int i=0; i<height-1; ++i) for(int j=0; j<width-1; ++j)
        {
            auto const& center = sorted_matrix[height - i - 1][width + j - 1];
            auto const& upper  = sorted_matrix[height - i - 2][width + j - 1];
            auto const& right  = sorted_matrix[height - i - 1][width + j    ];

            if(exists(center) && exists(upper) && exists(right))
                sorted_matrix[height - i - 2][width + j] = ur_choose(comp_, upper, center, right);
            else
                break;
        }
        //中心を除き上に向かってループ，左に見ていく
        for(int i=0; i<height-1; ++i) for(int j=0; j<width-1; ++j)
        {
            auto const& center = sorted_matrix[height - i - 1][width - j - 1];
            auto const& upper  = sorted_matrix[height - i - 2][width - j - 1];
            auto const& left   = sorted_matrix[height - i - 1][width - j - 2];

            if(exists(center) && exists(upper) && exists(left))
                sorted_matrix[height - i - 2][width - j - 2] = ul_choose(comp_, upper,left, center);
            else
                break;
        }
        //中心を除き下に向かってループ，右に見ていく
        for(int i=0; i<height-1; ++i) for(int j=0; j<width-1; ++j)
        {
            auto const& center = sorted_matrix[height + i - 1][width + j - 1];
            auto const& lower  = sorted_matrix[height + i    ][width + j - 1];
            auto const& right  = sorted_matrix[height + i - 1][width + j    ];
            
            if(exists(center) && exists(lower) && exists(right))
                sorted_matrix[height + i][width + j] = dr_choose(comp_, center, right, lower);
            else
                break;
        }
        //中心を除き下に向かってループ，左に見ていく
        for(int i=0; i<height-1; ++i) for(int j=0; j<width-1; ++j)
        {
            auto const& center = sorted_matrix[height + i - 1][width - j - 1];
            auto const& lower  = sorted_matrix[height + i    ][width - j - 1];
            auto const& left   = sorted_matrix[height + i + j][width - j - 2];
            
            if(exists(center) && exists(lower) && exists(left))
                sorted_matrix[height + i][width + j] = dl_choose(comp_, left, center, lower);
            else
                break;
        }

        std::vector<std::vector<point_type>> one_answer(
            height,
            std::vector<point_type>(width)
            );
        
        for(int y=0; y<height; ++y) for(int x=0; x<width; ++x)
        {
            if(array_sum(sorted_matrix, x, y, height, width) == ((width*height-1)*(width*height)/2))
            {
                for(int i=0; i<height; ++i) for(int j=0; j<width; ++j)
                {
                    one_answer[i][j] = sorted_matrix[y + i][x + j];
                }
                answer.push_back(std::move(one_answer));
            }
        }
    }

	//現段階で重複しているものは1つに絞る
	// unique()を使う準備としてソートが必要
	std::sort(answer.begin(), answer.end());
	// unique()をしただけでは後ろにゴミが残るので、eraseで削除する
	answer.erase(std::unique(answer.begin(), answer.end()), answer.end());

//
//yrange2.5
//
	

#ifdef _DEBUG
    std::cout << "There are " << answer.size() << " solutions" << std::endl;
    for(auto const& one_answer : answer)
    {
        for(int i=0; i<one_answer.size(); ++i)
        {
            for(int j=0; j<one_answer.at(0).size(); ++j)
            {
                auto const& data = one_answer[i][j];
                std::cout << boost::format("(%2d,%2d) ") % data.x % data.y;
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
#endif

    return answer;
}

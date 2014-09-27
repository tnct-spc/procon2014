﻿#include <limits>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <sort_algorithm/compare.hpp>
#include <data_type.hpp>
#include <splitter.hpp>

point_type ur_choose(compared_type const& comp, point_type const ul, point_type const dl, point_type const dr)
{
    point_type possition;
    uint_fast64_t sum = std::numeric_limits<uint_fast64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        if(comp[dr.y][dr.x][i][j].up == std::numeric_limits<uint_fast64_t>::max()
            || comp[ul.y][ul.x][i][j].right == std::numeric_limits<uint_fast64_t>::max()) continue;

        uint_fast64_t const tmp = comp[dr.y][dr.x][i][j].up + comp[ul.y][ul.x][i][j].right;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

point_type ul_choose(compared_type const& comp, point_type const ur, point_type const dl, point_type const dr)
{
    point_type possition;
    uint_fast64_t sum = std::numeric_limits<uint_fast64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        if(comp[dl.y][dl.x][i][j].up == std::numeric_limits<uint_fast64_t>::max()
            || comp[ur.y][ur.x][i][j].left == std::numeric_limits<uint_fast64_t>::max()) continue;

        uint_fast64_t const tmp = comp[dl.y][dl.x][i][j].up + comp[ur.y][ur.x][i][j].left;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

point_type dr_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dl)
{
    point_type possition;
    uint_fast64_t sum = std::numeric_limits<uint_fast64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        if(comp[ur.y][ur.x][i][j].down == std::numeric_limits<uint_fast64_t>::max()
            || comp[dl.y][dl.x][i][j].right == std::numeric_limits<uint_fast64_t>::max()) continue;

        uint_fast64_t const tmp = comp[ur.y][ur.x][i][j].down + comp[dl.y][dl.x][i][j].right;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

point_type dl_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dr)
{
    point_type possition;
    uint_fast64_t sum = std::numeric_limits<uint_fast64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        if(comp[ul.y][ul.x][i][j].down == std::numeric_limits<uint_fast64_t>::max()
            || comp[dr.y][dr.x][i][j].left == std::numeric_limits<uint_fast64_t>::max()) continue;

        uint_fast64_t const tmp = comp[ul.y][ul.x][i][j].down + comp[dr.y][dr.x][i][j].left;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

/*場を評価する関数*/
uint_fast64_t form_evaluate(compared_type const& comp_, std::vector<std::vector<point_type> > const& matrix)
{
	uint_fast64_t s = 0;
	const int width = matrix[0].size();
	const int height = matrix.size();

	for (int i = 0; i < height; ++i)for (int j = 0; j < width; ++j){
		if (i != height - 1) s += comp_[matrix[i][j].y][matrix[i][j].x][matrix[i + 1][j].y][matrix[i + 1][j].x].down;
		if (j != width - 1) s += comp_[matrix[i][j].y][matrix[i][j].x][matrix[i][j + 1].y][matrix[i][j + 1].x].right;
	}
	return s;
}

/*指定された範囲内の問題画像の種類を返す関数*/
int get_kind_num(question_raw_data const& data_,std::vector<std::vector<point_type>> const& matrix, int const x, int const y)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;

	std::vector<point_type> temp;
	for (int i = 0; i < sepy; i++){
		for (int j = 0; j < sepx; j++){
			temp.push_back(matrix[y + i][x + j]);
		}
	}
	std::sort(temp.begin(), temp.end());
	temp.erase(std::unique(temp.begin(), temp.end()), temp.end());
	return temp.size();
}

/*指定された範囲内の場を評価する関数*/
uint_fast64_t range_evaluate(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type> > matrix, int x, int y)
{
	uint_fast64_t s = 0;
	for (int i = 0; i < data_.split_num.second - 1; i++)for (int j = 0; j < data_.split_num.first - 1; j++){
		s += comp_[matrix[y + i][x + j].y][matrix[y + i][x + j].x][matrix[y + i][x + j + 1].y][matrix[y + i][x + j + 1].x].right;
		s += comp_[matrix[y + i][x + j].y][matrix[y + i][x + j].x][matrix[y + j][x + i + 1].y][matrix[y + j][x + i + 1].x].down;
	}
	return s;
}

/*並べられたそれぞれの問題画像の持つrgb値を調べる関数*/
std::vector<point_score> make_matrix_rgb_database(question_raw_data const& data_, compared_type const& comp_, std::vector<std::vector<point_type> >& matrix)
{
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;
	std::vector<point_score> point_score_vec;
	std::vector<std::vector<int>>data;
	data.resize(sepy);
	for (auto& temp : data) temp.resize(sepx);

	for (int i = 0; i < sepy; i++)for (int j = 0; j < sepx - 1; j++)data[i][j] = data[i][j + 1] = comp_[matrix[i][j].y][matrix[i][j].x][matrix[i][j + 1].y][matrix[i][j + 1].x].right;//横加算
	for (int i = 0; i < sepy - 1; i++)for (int j = 0; j < sepx; j++)data[i][j] = data[i + 1][j] = comp_[matrix[i][j].y][matrix[i][j].x][matrix[i + 1][j].y][matrix[i + 1][j].x].down;//縦加算

	data[0][0] /= 2;//左上隅
	data[0][sepx - 1] /= 2;//右上隅
	data[sepy - 1][0] /= 2;//左下隅
	data[sepy - 1][sepx - 1] /= 2;//右下隅

	for (int j = 0; j < sepx - 2; j++)data[0][j + 1] /= 3;//上辺除算
	for (int j = 0; j < sepx - 2; j++)data[sepx - 1][j + 1] /= 3;//下辺除算
	for (int i = 0; i < sepy - 2; i++)data[i + 1][0] /= 3;//左辺除算	
	for (int i = 0; i < sepy - 2; i++)data[i + 1][sepy - 1] /= 3;//右辺除算

	for (int i = 1; i < sepy - 1; i++)for (int j = 1; j < sepx - 1; j++)data[i][j] /= 4;//中央除算

	for (int i = 0; i < sepy; ++i)for (int j = 0; j < sepx; ++j){
		point_score_vec.push_back(std::move(point_score{ { j, i }, data[i][j] }));
	}

	std::sort(point_score_vec.rbegin(), point_score_vec.rend());

	return(std::move(point_score_vec));
}

//渡されたpoint_scoreの平均値を返す
int get_matrix_average(std::vector<point_score>const& matrix)
{
	uint_fast64_t average = 0;
	for (auto temp:matrix){
		average += temp.score;
	}
	return average / matrix.size();
}

//matrixの重複しているものを{width,height}に置き換える
void duplicate_delete(compared_type const& comp_, std::vector<std::vector<point_type> >& matrix)
{
	int const sepx = matrix.at(0).size();
	int const sepy = matrix.size();
	struct overlap_set{
		uint_fast64_t score;
		point_type point,target;
		bool operator<(const overlap_set& right) const {
			return (score == right.score) ? point < right.point : score < right.score;
		}
	};

	std::vector<overlap_set> overlap(sepx*sepy);
	std::vector<std::vector<overlap_set> > each_overlap(sepx*sepy);

	//リストにする
	for (int i = 0; i < sepy; i++) for (int j = 0; j < sepx; j++){
		overlap[sepy*i + j].target = matrix[j][i];
		overlap[sepy*i + j].point = point_type{ j, i };
		overlap[sepy*i + j].score = one_side_val(comp_, matrix, point_type{ j, i });
	}

	//リストを分割画像ごとに分類
	for (int i = 0; i < sepx*sepy; i++){
		each_overlap[overlap[i].point.y / sepy + overlap[i].point.x].push_back(overlap[i]);
	}

	//リスト内で一番マッチしているもの見つける それ以外のものは333に置き換え
	//重複がない場合とその問題画像が使われていない場合はcontinue
	point_type temp;
	std::list <point_type> target(sepx*sepy);
	for (int i = 0; i < sepx*sepy; i++)target.push_back(point_type{ i%sepx, i/sepy });

	for (int i = 0; i < sepx*sepy; i++){
		if (each_overlap[i].size() == 0)continue;
		if (each_overlap[i].size() == 1){

			temp = each_overlap[i][0].target;
			target.remove_if([temp](point_type re) { return re == temp; });
			continue;
		}
		std::sort(each_overlap[i].begin(), each_overlap[i].end());
		temp = each_overlap[i][0].point;
		target.remove_if([temp](point_type re) { return re == temp; });
		for (unsigned int u = 1; u < each_overlap[i].size(); u++){
			matrix[each_overlap[i][u].point.x][each_overlap[i][u].point.y] = point_type{ sepx, sepy };
		}
	}
}

/*1辺あたりのdifference of rgb dataを返す関数*/
int one_side_val(compared_type const& comp_, std::vector<std::vector<point_type> >matrix, point_type point)
{
	point_type invalid_val = { matrix.at(0).size(), matrix.size() };
	int const sepx = matrix.at(0).size();
	int const sepy = matrix.size();
	uint_fast64_t s = 0;
	int count = 0;
	//上
	if (point.y - 1 >= 0 && matrix[point.y - 1][point.x] != invalid_val){
		s += comp_[matrix[point.y][point.x].y][matrix[point.y][point.x].x][matrix[point.y - 1][point.x].y][matrix[point.y - 1][point.x].x].up;
		count++;
	}
	//下
	if (point.y + 1 < sepy && matrix[point.y + 1][point.x] != invalid_val){
		s += comp_[matrix[point.y][point.x].y][matrix[point.y][point.x].x][matrix[point.y + 1][point.x].y][matrix[point.y + 1][point.x].x].down;
		count++;
	}
	//左
	if (point.x - 1 >= 0 && matrix[point.y][point.x - 1] != invalid_val){
		s += comp_[matrix[point.y][point.x].y][matrix[point.y][point.x].x][matrix[point.y][point.x - 1].y][matrix[point.y][point.x - 1].x].left;
		count++;
	}
	//右
	if (point.x + 1 < sepx && matrix[point.y][point.x + 1] != invalid_val){
		s += comp_[matrix[point.y][point.x].y][matrix[point.y][point.x].x][matrix[point.y][point.x + 1].y][matrix[point.y][point.x + 1].x].right;
		count++;
	}
	return s / count;
}

//与えられたsorted_matrixに与えられたmatrixを与えられた点からコピーしようと試みる
int try_matrix_copy(std::vector<std::vector<point_type>>& sorted_matrix, std::vector<std::vector<point_type>>const& matrix, point_type point)
{
	int const sepx = sorted_matrix.at(0).size();
	int const sepy = sorted_matrix.size();
	point_type const invalid_val = { sepx, sepy };

	for (int k = 0; k < sepy; ++k)for (int l = 0; l < sepx; ++l){
		if (matrix[k][l] == invalid_val) continue;
		if (point.x + l >= sepx || point.y + k >= sepy)return -1;
		sorted_matrix[point.y + k][point.x + l] = matrix[k][l];
	}
	return 0;
}

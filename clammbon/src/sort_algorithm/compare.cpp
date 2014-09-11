#include <limits>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <sort_algorithm/compare.hpp>
#include <data_type.hpp>
#include <splitter.hpp>

point_type ur_choose(compared_type const& comp, point_type const ul, point_type const dl, point_type const dr)
{
    point_type possition;
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[dr.y][dr.x][i][j].up + comp[ul.y][ul.x][i][j].right;
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
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[dl.y][dl.x][i][j].up + comp[ur.y][ur.x][i][j].left;
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
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[ur.y][ur.x][i][j].down + comp[dl.y][dl.x][i][j].right;
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
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[ul.y][ul.x][i][j].down + comp[dr.y][dr.x][i][j].left;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

//#########################################################################################
//新しい<s>ipad</s>

point_type ur_choose(compared_type const& comp,cr_set const& cr,int method, point_type const ul, point_type const dl, point_type const dr)
{
	//column
	cv::Point column_mp;
	double column_mv;
	//結果を格納する画像リソースを確保
	cv::Mat column_ccoeff = cvCreateImage(cvSize(cr.column.rows, 1), IPL_DEPTH_32F, 1);
	//テンプレート・マッチングにより計算
	cv::matchTemplate(cr.column, cr.each_direction[right].at(ul),column_ccoeff,method);
	//最小値・最大値とその座標を抽出
	cv::minMaxLoc(column_ccoeff, NULL, &column_mv, NULL, &column_mp, NULL);
	std::cout << "x = " << column_mp.x << " y = " << column_mp.y << " score = " << column_mp << std::endl;
	
	//row
	cv::Point row_mp;
	double row_mv;
	//結果を格納する画像リソースを確保
	cv::Mat row_ccoeff = cvCreateImage(cvSize(cr.column.rows, 1), IPL_DEPTH_32F, 1);
	//テンプレート・マッチングにより計算
	cv::matchTemplate(cr.row, cr.each_direction[up].at(dr), row_ccoeff, method);
	//最小値・最大値とその座標を抽出
	cv::minMaxLoc(row_ccoeff, NULL, &column_mv, NULL, &column_mp, NULL);
	std::cout << "x = " << row_mp.x << " y = " << row_mp.y << " score = " << row_mp << std::endl;













	point_type possition;
	possition = { 0, 0 };
	/*
	uint64_t sum = std::numeric_limits<uint64_t>::max();

	for (int i = 0; i<comp.size(); ++i) for (int j = 0; j<comp[0].size(); ++j)
	{
		uint64_t const tmp = comp[dr.y][dr.x][i][j].up + comp[ul.y][ul.x][i][j].right;
		if (0 < tmp && tmp < sum)
		{
			sum = tmp;
			possition = { j, i };
		}
	}
	*/
	return possition;
}
/*
point_type ul_choose(compared_type const& comp, point_type const ur, point_type const dl, point_type const dr)
{
	point_type possition;
	uint64_t sum = std::numeric_limits<uint64_t>::max();

	for (int i = 0; i<comp.size(); ++i) for (int j = 0; j<comp[0].size(); ++j)
	{
		uint64_t const tmp = comp[dl.y][dl.x][i][j].up + comp[ur.y][ur.x][i][j].left;
		if (0 < tmp && tmp < sum)
		{
			sum = tmp;
			possition = { j, i };
		}
	}

	return possition;
}

point_type dr_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dl)
{
	point_type possition;
	uint64_t sum = std::numeric_limits<uint64_t>::max();

	for (int i = 0; i<comp.size(); ++i) for (int j = 0; j<comp[0].size(); ++j)
	{
		uint64_t const tmp = comp[ur.y][ur.x][i][j].down + comp[dl.y][dl.x][i][j].right;
		if (0 < tmp && tmp < sum)
		{
			sum = tmp;
			possition = { j, i };
		}
	}

	return possition;
}

point_type dl_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dr)
{
	point_type possition;
	uint64_t sum = std::numeric_limits<uint64_t>::max();

	for (int i = 0; i<comp.size(); ++i) for (int j = 0; j<comp[0].size(); ++j)
	{
		uint64_t const tmp = comp[ul.y][ul.x][i][j].down + comp[dr.y][dr.x][i][j].left;
		if (0 < tmp && tmp < sum)
		{
			sum = tmp;
			possition = { j, i };
		}
	}

	return possition;
}
*/
//#########################################################################################


/*場を評価する関数*/
uint64_t form_evaluate(compared_type const& comp_, std::vector<std::vector<point_type> > const& matrix)
{
	uint64_t s = 0;
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


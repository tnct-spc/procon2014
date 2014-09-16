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

//method = CV_TM_SQDIFF, CV_TM_CCORR_NORMED, CV_TM_CCORR, CV_TM_CCORR_NORMED, CV_TM_CCOEFF, CV_TM_CCOEFF_NORMED
point_type ur_choose(compared_type const& comp,cr_set const& cr,int method, point_type const ul, point_type const dl, point_type const dr)
{
	int const sepx = cr.row.rows / 6;
	int const sepy = cr.column.cols / 6	;

	std::cout << "sepx = " << sepx << " sepy =" << sepy << std::endl;

	cv::Mat column_grey = cr.column.reshape(1);
	cv::Mat one_column_grey = cr.each_direction[right].at(ul).reshape(1);

	cv::Point column_max_p, column_min_p;
	double column_max_v = 0;
	double column_min_v = 0;

	cv::Point row_max_p, row_min_p;
	double row_max_v = 0;
	double row_min_v = 0;

	//結果を格納する画像リソースを確保
	cv::Mat column_ccoeff = cvCreateImage(cvSize(cr.column.rows, 1), IPL_DEPTH_32F, 1);
	cv::Mat row_ccoeff = cvCreateImage(cvSize(cr.column.rows, 1), IPL_DEPTH_32F, 1);

	switch (method)
	{
	case CV_TM_SQDIFF:
	case CV_TM_SQDIFF_NORMED:
	//小さいほうが良い
		//column
		//cv::matchTemplate(column_grey, one_column_grey, column_ccoeff, method);
		cv::matchTemplate(cr.column, cr.each_direction[right].at(ul), column_ccoeff, method);
		cv::minMaxLoc(column_ccoeff, &column_min_v,NULL, &column_min_p, NULL);
		std::cout <<  "column_min_p = " << column_min_p << " column_min_v = " << column_min_v << std::endl;
		std::cout << "## = " << column_min_p.x / 2 /sepx <<  column_min_p.x %(2*sepx)/2 << std::endl;
		cv::namedWindow("test1");
		cv::imshow("test1", cr.column);
		cv::namedWindow("test2");
		cv::imshow("test2", cr.each_direction[right].at(ul));


		//row
		cv::matchTemplate(cr.row, cr.each_direction[up].at(dr), row_ccoeff, method);
		cv::minMaxLoc(row_ccoeff, &row_min_v, NULL, &row_min_p, NULL);
		std::cout << "row_min_p = " << row_min_p << " row_min_v = " << row_min_v << std::endl;
		std::cout << "## = " << row_min_p.y / 2 / sepx << row_min_p.y % (2 * sepx) / 2 << std::endl;	
		cv::namedWindow("test3");
		cv::imshow("test3", cr.row);
		cv::namedWindow("test4");
		cv::imshow("test4", cr.each_direction[up].at(dr));

		break;
	case CV_TM_CCORR:
	case CV_TM_CCORR_NORMED:
	case CV_TM_CCOEFF:
	case CV_TM_CCOEFF_NORMED:
		//0から遠いほど良い
		//column
		//cv::matchTemplate(column_grey, one_column_grey, column_ccoeff, method);
		cv::matchTemplate(cr.column, cr.each_direction[right].at(ul), column_ccoeff, method);
		cv::minMaxLoc(column_ccoeff, NULL, &column_max_v, NULL, &column_max_p);
		std::cout << "column_max_p = " << column_max_p << " column_max_v = " << column_max_v << std::endl;

		//row
		cv::matchTemplate(cr.row, cr.each_direction[up].at(dr), row_ccoeff, method);
		cv::minMaxLoc(row_ccoeff,NULL, &row_max_v, NULL, &row_max_p);
		std::cout << "row_max_p = " << row_max_p << " row_max_v = " << row_max_v << std::endl;

		break;
	}

	cv::waitKey(0);

	return{ 0, 0 };
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


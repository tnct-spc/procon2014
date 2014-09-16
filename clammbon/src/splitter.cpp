#include <limits>
#include <iostream>
#include <vector>
#include <map>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "splitter.hpp"
#include "data_type.hpp"

split_image_type splitter::split_image(question_raw_data const& raw) const
{
    int const parts_width = raw.size.first / raw.split_num.first;
    int const parts_height = raw.size.second / raw.split_num.second;

    split_image_type split_pixels;
    split_pixels.reserve(raw.split_num.second);
    
    for(int i=0; i<raw.split_num.second; ++i)
    {
        std::vector<image_type> line;
        line.reserve(raw.split_num.first);

        for(int j=0; j<raw.split_num.first; ++j)
        {
            cv::Rect roi(
                parts_width*j, parts_height*i,
                parts_width  , parts_height
                );

            line.push_back(raw.pixels(roi).clone());
        }

        split_pixels.push_back(std::move(line));
    }

    return split_pixels;
}

image_type splitter::join_image(split_image_type const& source) const
{
    int const part_width   = source[0][0].cols;
    int const part_height  = source[0][0].rows;
    int const split_width  = source[0].size();
    int const split_height = source.size();
    int const join_width   = part_width  * split_width;
    int const join_height  = part_height * split_height;

    image_type joined(join_height, join_width, CV_8UC3);
    for(int i=0; i<split_height; ++i)
    {
        for(int j=0; j<split_width; ++j)
        {
            cv::Rect roi(
                part_width*j, part_height*i,
                part_width  , part_height
                );

            source[i][j].copyTo(joined(roi));
        }
    }

    return joined;
}

unfold_image_type splitter::unfold_image(image_type const& fold_image) const
{
    int const width  = fold_image.cols;
    int const height = fold_image.rows;

    unfold_image_type unfold_image;
    unfold_image.reserve(3 * height * width);

    for(auto const& pixel : cv::Mat_<cv::Vec3b>(fold_image))
    {
        unfold_image.push_back(pixel[2]); // R
        unfold_image.push_back(pixel[1]); // G
        unfold_image.push_back(pixel[0]); // B
    }
    
    return unfold_image;
}

image_type splitter::fold_image(unfold_image_type const& unfold_image, int const height, int const width) const
{
    image_type fold_image(height, width, CV_8UC3);

    auto it = unfold_image.begin();
    for(auto& pixel : cv::Mat_<cv::Vec3b>(fold_image))
    {
        pixel[2] = *it++; // R
        pixel[1] = *it++; // G
        pixel[0] = *it++; // B
    }

    assert(it == unfold_image.end());

    return fold_image;
}

//column_row_set作成関数
cr_set splitter::make_column_row_set(question_raw_data const& data_) const
{
	cr_set answer;
	int const parts_width = data_.size.first / data_.split_num.first;
	int const parts_height = data_.size.second / data_.split_num.second;
	int const sepx = data_.split_num.first;
	int const sepy = data_.split_num.second;

	std::vector<std::map<point_type, cv::Mat>> each_direction(4);

	splitter sp;//どこからか持ってきてたsplitter
	split_image_type splitted = sp.split_image(data_);

	cv::Rect roi_row_rect(0, 0, parts_width, 1);
	cv::Mat combined_row_img(cv::Size(parts_width, sepx*sepy * 2), CV_8UC3);
	cv::Rect roi_column_rect(0, 0, 1, parts_height);
	cv::Mat combined_column_img(cv::Size(sepx*sepy * 2, parts_height), CV_8UC3);

	for (int i = 0; i < sepy; i++){
		for (int j = 0; j < sepx; j++){
			point_type const now_point = { j, i };

			cv::Mat roi_1(combined_row_img, roi_row_rect);

			cv::Rect up_roi(0, 0, parts_width, 1);
			cv::Mat up_mat(splitted[i][j], up_roi);
			each_direction[up].insert(std::make_pair(now_point, up_mat));
			up_mat.copyTo(roi_1);
			roi_row_rect.y += 1;

			cv::Rect down_roi(0, parts_height - 1, parts_width, 1);
			cv::Mat down_mat(splitted[i][j], down_roi);
			each_direction[down].insert(std::make_pair(now_point, down_mat));
			down_mat.copyTo(roi_1);
			roi_row_rect.y += 1;

			cv::Mat roi_2(combined_column_img, roi_column_rect);

			cv::Rect left_roi(0, 0, 1, parts_height);
			cv::Mat left_mat(splitted[i][j], left_roi);
			each_direction[left].insert(std::make_pair(now_point, left_mat));
			left_mat.copyTo(roi_2);
			roi_column_rect.x += 1;

			cv::Rect right_roi(parts_width - 1, 0, 1, parts_height);
			cv::Mat right_mat(splitted[i][j], right_roi);
			each_direction[right].insert(std::make_pair(now_point, right_mat));
			right_mat.copyTo(roi_2);
			roi_column_rect.x += 1;
		}
	}

	answer.column = combined_column_img;
	answer.row = combined_row_img;
	answer.each_direction = each_direction;

	return answer;
}

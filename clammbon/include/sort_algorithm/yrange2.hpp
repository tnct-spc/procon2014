#ifndef CLAMMBON_SORT_YRANGE2_HPP
#define CLAMMBON_SORT_YRANGE2_HPP

#include <vector>
#include <data_type.hpp>
#include <omp.h>

class yrange2
{
private:
    typedef std::vector<std::vector<point_type>> return_type;
    
    question_raw_data const& data_;
    compared_type const& comp_;
    adjacent_type const adjacent_data_;
    std::vector<std::vector<std::vector<point_type>>> sorted_matrix_;
	omp_lock_t ol;

    // 2値座標系式から1値座標系式に変えながら和
    // 指定した範囲の配列の和を返す
    int array_sum(return_type const& array_, int const x, int const y, int const height, int const width) const;

	//cv::Matの塊にする
	cv::Mat combine_image(std::vector<std::vector<point_type>> matrix);
	
public:
    // 泣きのポインタ渡し
    yrange2(question_raw_data const& data, compared_type const& comp);
    virtual ~yrange2() = default;

    std::vector<answer_type_y> operator() ();
    std::vector<std::vector<std::vector<point_type>>> const& sorted_matrix();
};

#endif

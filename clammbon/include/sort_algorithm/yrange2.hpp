#ifndef CLAMMBON_SORT_YRANGE2_HPP
#define CLAMMBON_SORT_YRANGE2_HPP

#include <vector>
#include <data_type.hpp>

class yrange2
{
private:
    typedef std::vector<std::vector<point_type>> return_type;
    
    question_raw_data const& data_;
    compared_type const& comp_;
    adjacent_type const adjacent_data_;

    // 2値座標系式から1値座標系式に変えながら和
    // 指定した範囲の配列の和を返す
    int array_sum(return_type const& array_, int const x, int const y, int const height, int const width) const;

	//場を評価する関数
	uint64_t yrange2::form_evaluate(std::vector<point_type> matrix);
	//縦入れ替え
	void yrange2::column_replacement(return_type& matrix);
	//横入れ替え
	void yrange2::row_replacement(return_type& matrix);
	//指定された範囲内の問題画像の種類を返す関数
	int get_kind_num(std::vector<point_type> matrix, int x, int y);

public:
    // 泣きのポインタ渡し
    yrange2(question_raw_data const& data, compared_type const& comp);
    virtual ~yrange2() = default;

    std::vector<std::vector<std::vector<point_type>>> operator() ();
};

#endif

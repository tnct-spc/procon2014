#ifndef CLAMMBON_SORT_YRANGE5_HPP
#define CLAMMBON_SORT_YRANGE5_HPP

#include <vector>
#include <data_type.hpp>

class yrange5
{
private:
    typedef std::vector<std::vector<point_type>> return_type;
    
	//kind_rgb用構造体
	struct kind_rgb{
		int x, y, kind; 
		uint_fast64_t score;
		bool operator<(const kind_rgb& right) const {
			return score == right.score ? x < right.x : score < right.score;
		}
	};

    question_raw_data const& data_;
    compared_type const& comp_;
    adjacent_type const adjacent_data_;

    // 2値座標系式から1値座標系式に変えながら和
    // 指定した範囲の配列の和を返す
    int array_sum(return_type const& array_, int const x, int const y, int const height, int const width) const;

	//縦入れ替え
	void column_replacement(answer_type_y& matrix);
	//横入れ替え
	void row_replacement(answer_type_y& matrix);
	//cv::Matの塊にする
	cv::Mat combine_image(answer_type_y const & matrix);
	
public:
    // 泣きのポインタ渡し
    yrange5(question_raw_data const& data, compared_type const& comp);
    virtual ~yrange5() = default;

    std::vector<answer_type_y> operator() (std::vector<std::vector<std::vector<point_type>>> const& sorted_matrix);
};

#endif

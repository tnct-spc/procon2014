#ifndef RESOLVER_SPLITTER_HPP
#define RESOLVER_SPLITTER_HPP

#include "data_type.hpp"

class splitter
{
public:
    explicit splitter() = default;
    virtual ~splitter() = default;

    //イメージ分割関数
    split_image_type split_image(question_raw_data const& raw) const;

	//ぼかした分割画像
	split_image_type splitter::split_image_gaussianblur(question_raw_data const& raw) const;

	//cv::arcLength用グレースケールで読み込んでエッジ検出してmatにかためて返す関数
	cv::Mat combine_grey(question_raw_data const& raw, std::vector<std::vector<point_type>> const& matrix)const;

    //イメージ合成関数(それぞれのパーツが同じ大きさであると仮定する)
    image_type join_image(split_image_type const& source) const;

    //イメージ展開関数
    unfold_image_type unfold_image(image_type const& fold_image) const;

    //イメージ圧縮関数
    image_type fold_image(unfold_image_type const& unfold_image, int const height, int const width) const;


};

#endif

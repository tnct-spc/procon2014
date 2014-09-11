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

    //イメージ合成関数(それぞれのパーツが同じ大きさであると仮定する)
    image_type join_image(split_image_type const& source) const;

    //イメージ展開関数
    unfold_image_type unfold_image(image_type const& fold_image) const;

    //イメージ圧縮関数
    image_type fold_image(unfold_image_type const& unfold_image, int const height, int const width) const;

	//column_set作成関数
	cr_set make_column_row_set(question_raw_data const& data_) const;

};

#endif

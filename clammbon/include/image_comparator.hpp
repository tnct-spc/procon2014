#ifndef RESOLVER_IMAGE_COMPARATOR_HPP
#define RESOLVER_IMAGE_COMPARATOR_HPP

#include "data_type.hpp"

class image_comparator
{
public:
    // 座標(j,i)と座標(l,k)の比較結果としては，compared_type[i][j][k][l]にtuple<上,右,下,左>で入る
    typedef std::vector<std::vector<std::vector<std::vector<direction_type<uint_fast64_t>>>>> compared_type;

    image_comparator() = default;
    virtual ~image_comparator() = default;
    
protected:
    //2つのピクセル間の距離を2乗した値を返却．result = |r| + |g| + |b|
    virtual int pixel_comparison(pixel_type const& lhs, pixel_type const& rhs) const;

    //2つのピクセル列を比較した値を返す
    virtual int pixel_line_comparison(image_type const& lhs, image_type const& rhs) const;

    //一枚目の(r|l|u|d)と二枚目の(l|r|d|u)を見る関数, 一致が多いほど低いを返す
    virtual uint_fast64_t rl_comparison(image_type const& lhs, image_type const& rhs) const;
    virtual uint_fast64_t lr_comparison(image_type const& lhs, image_type const& rhs) const;
    virtual uint_fast64_t ud_comparison(image_type const& lhs, image_type const& rhs) const;
    virtual uint_fast64_t du_comparison(image_type const& lhs, image_type const& rhs) const;

public:
    virtual compared_type image_comp(question_raw_data const& data_,split_image_type const& image) const;
};

class image_comparator_dx : public image_comparator
{
public:
	image_comparator_dx() = default;
	virtual ~image_comparator_dx() = default;

protected:
	//2つのピクセル間の距離を2乗した値を返却．result = sqrt(r^2 + g^2 + b^2)
    virtual int pixel_comparison(pixel_type const& lhs, pixel_type const& rhs) const override;
};

#endif

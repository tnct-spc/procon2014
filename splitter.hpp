#include "data_type.hpp"

class splitter
{
public:
    explicit splitter() = default;
    virtual ~splitter() = default;

    question_data operator() (question_raw_data const& raw) const;

private:
    //nまでの和を求める関数
    inline int n_sum(int n) const;

    //イメージ分割関数
    split_image_type split_image(question_raw_data const& raw) const;

    //pixel比較
    int pixel_comparison(pixel_type const& lhs, pixel_type const& rhs) const;

    //一枚目の右端と二枚目の左端を見る関数, 一致が多いほど低いを返す
    uint64_t rl_comparison(image_type const& lhs, image_type const& rhs) const;

    //一枚目の左端と二枚目の右端を見る関数, 一致が多いほど低いを返す
    uint64_t lr_comparison(image_type const& lhs, image_type const& rhs) const;

    //一枚目の上端と二枚目の下端を見る関数, 一致が多いほど低いを返す
    uint64_t ud_comparison(image_type const& lhs, image_type const& rhs) const;

    //一枚目の下端と二枚目の上端を見る関数, 一致が多いほど低いを返す
    uint64_t du_comparison(image_type const& lhs, image_type const& rhs) const;

    compared_type image_comp(split_image_type const& image) const;

};

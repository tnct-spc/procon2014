#include "data_type.hpp"

class pixel_sorter
{
public:
    explicit pixel_sorter() = default;
    virtual ~pixel_sorter() = default;

    question_data operator() (question_raw_data const& raw) const;

private:
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


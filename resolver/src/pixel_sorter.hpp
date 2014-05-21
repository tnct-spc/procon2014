#ifndef RESOLVER_PIXEL_SORTER_HPP
#define RESOLVER_PIXEL_SORTER_HPP

#include "data_type.hpp"
#include "splitter.hpp"

class pixel_sorter
{
public:
    // 座標(j,i)と座標(l,k)の比較結果としては，compared_type[i][j][k][l]にtuple<上,右,下,左>で入る
    // compared_typeより，一番距離が近い断片画像をadjacent_typeに整理する
    typedef std::vector<std::vector<std::vector<std::vector<direction_type<uint64_t>>>>> compared_type;
    typedef std::vector<std::vector<direction_type<point_type>>> adjacent_type;

    explicit pixel_sorter() = default;
    virtual ~pixel_sorter() = default;

    typedef std::vector<std::vector<point_type>> return_type;
    return_type operator() (question_raw_data const& raw) const;
    
    std::vector<return_type> yrange2(const int width, const int height, adjacent_type const& adjacent_data, compared_type const& compared_data) const;

private:
    // それぞれ，右上，左上，右下，左下を探索する関数
    point_type ur_choose(compared_type const& comp, point_type const ul, point_type const dl, point_type const dr) const;
    point_type ul_choose(compared_type const& comp, point_type const ur, point_type const dl, point_type const dr) const;
    point_type dr_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dl) const;
    point_type dl_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dr) const;

    //pixelを比較し，RGB距離の2乗を返す
    int pixel_comparison(pixel_type const& lhs, pixel_type const& rhs) const;

    // 一致が多いほど低い値を返す(つまり画像の距離の2乗)
    // それぞれ，1枚目の右端と2枚目の左端，1枚目の左端と2枚目の右端，1枚目の上端と2枚目の下端，1枚目の下端と2枚目の上端を探索．
    uint64_t rl_comparison(image_type const& lhs, image_type const& rhs) const;
    uint64_t lr_comparison(image_type const& lhs, image_type const& rhs) const;
    uint64_t ud_comparison(image_type const& lhs, image_type const& rhs) const;
    uint64_t du_comparison(image_type const& lhs, image_type const& rhs) const;

    compared_type image_comp(split_image_type const& image) const;
    adjacent_type select_minimum(compared_type const& compared_data) const;
    int array_sum(return_type const& array_, int const x, int const y, int const height, int const width) const;

    splitter split_;
};

#endif

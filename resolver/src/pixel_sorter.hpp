#ifndef RESOLVER_PIXEL_SORTER_HPP
#define RESOLVER_PIXEL_SORTER_HPP

#include "data_type.hpp"

class pixel_sorter
{
public:
    
    // 驚きの気持ち悪さ
    // 座標(j,i)と座標(l,k)の比較結果としては，[i][j][k][l]にtuple<上,右,下,左>で入る
    typedef std::vector<std::vector<std::vector<std::vector<direction_type<uint64_t>>>>> compared_type;

    // その最小値が入る
    typedef std::vector<std::vector<direction_type<point_type>>> adjacent_type;

    explicit pixel_sorter() = default;
    virtual ~pixel_sorter() = default;

    std::vector<std::vector<point_type>> operator() (question_raw_data const& raw, split_image_type const& splited_image) const;
    
    std::vector<std::vector<std::vector<point_type>>> yrange2(const int width, const int height, adjacent_type const& adjacent_data, compared_type const& compared_data) const;

private:
    // それぞれ，右上，左上，右下，左下を探索する関数
    point_type ur_choose(compared_type const& comp, point_type const ul, point_type const dl, point_type const dr) const;
    point_type ul_choose(compared_type const& comp, point_type const ur, point_type const dl, point_type const dr) const;
    point_type dr_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dl) const;
    point_type dl_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dr) const;

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
    adjacent_type select_minimum(compared_type const& compared_data) const;
    int array_sum(std::vector<std::vector<point_type>> const& array_, int const i, int const j, int const height, int const width) const;
};

#endif

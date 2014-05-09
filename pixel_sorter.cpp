#include <algorithm>
#include <limits>
#include "pixel_sorter.hpp"

question_data pixel_sorter::operator() (question_raw_data const& raw) const
{
    question_data formed = {
        raw.split_num,
        raw.selectable_num,
        raw.cost.first,
        raw.cost.second,
        std::vector<std::vector<int>>(raw.split_num.second, std::vector<int>(raw.split_num.first, std::numeric_limits<int>::max()) )
    };

    auto& block = formed.block;

    //
    // Sub Algorithm
    // 正しい位置に並べた時に左上から，1~nまでの番号をふり，それが今どこにあるのかという情報をblockに格納
    //




    // Sub Algorithm End

    return formed;
}

int pixel_sorter::pixel_comparison(pixel_type const& lhs, pixel_type const& rhs) const
{
    // TODO: Memo: えっと，これはノルムになってないよね？(commented by: godai_0519)
    int s = 0;
    s += std::abs(std::get<0>(lhs) - std::get<0>(rhs)); //r
    s += std::abs(std::get<1>(lhs) - std::get<1>(rhs)); //g
    s += std::abs(std::get<2>(lhs) - std::get<2>(rhs)); //b
    return s;
}

//一枚目の右端と二枚目の左端を見る関数, 一致が多いほど低いを返す
uint64_t pixel_sorter::rl_comparison(image_type const& lhs, image_type const& rhs) const
{
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs[i].back(), rhs[i].front());
    return s;
}

//一枚目の左端と二枚目の右端を見る関数, 一致が多いほど低いを返す
uint64_t pixel_sorter::lr_comparison(image_type const& lhs, image_type const& rhs) const
{
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs[i].front(), rhs[i].back());
    return s;
}

//一枚目の上端と二枚目の下端を見る関数, 一致が多いほど低いを返す
uint64_t pixel_sorter::ud_comparison(image_type const& lhs, image_type const& rhs) const
{
    auto const& lhs_top    = lhs.front();
    auto const& rhs_bottom = rhs.back();
        
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs_top[i], rhs_bottom[i]);
    return s;
}

//一枚目の下端と二枚目の上端を見る関数, 一致が多いほど低いを返す
uint64_t pixel_sorter::du_comparison(image_type const& lhs, image_type const& rhs) const
{
    auto const& lhs_bottom = lhs.back();
    auto const& rhs_top    = rhs.front();
        
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs_bottom[i], rhs_top[i]);
    return s;
}

compared_type pixel_sorter::image_comp(split_image_type const& image) const
{
    //返却用変数
    //"最大距離が収納されたtuple"の4次元配列になる
    compared_type comp(
        image.size(),
        std::vector<std::vector<std::vector<std::tuple<uint64_t,uint64_t,uint64_t,uint64_t>>>>(
            image[0].size(),
            std::vector<std::vector<std::tuple<uint64_t,uint64_t,uint64_t,uint64_t>>>(
                image.size(),
                std::vector<std::tuple<uint64_t,uint64_t,uint64_t,uint64_t>>(
                    image[0].size(),
                    std::make_tuple(
                        std::numeric_limits<uint64_t>::max(),
                        std::numeric_limits<uint64_t>::max(),
                        std::numeric_limits<uint64_t>::max(),
                        std::numeric_limits<uint64_t>::max()
                        )
                    )
                )
            )
        );

    for(int i=0; i<image.size(); ++i) for(int j=0; j<image[0].size(); ++j)
    {
        for(int k=0; k<image.size(); ++k) for(int l=0; l<image[0].size(); ++l)
        {
            if(k > i || (k == i && l > j)) // (j,i)より(l,k)の方が，探索順として後半に或るための条件
            {
                //Note: 例えばCSSでは上右下左だけれども，右左上下でいいの(commented by: godai_0519)
                
                //綺麗に書けないわけだけれど，
                //順序を変えて逆の逆の組み合わせの相対評価は同じであることを使用して探索量を半分にする
                //例えば，Aから見たBは上なら，Bから見たAは下．
                std::tie(
                    std::get<0>(comp[i][j][k][l]),
                    std::get<1>(comp[i][j][k][l]),
                    std::get<2>(comp[i][j][k][l]),
                    std::get<3>(comp[i][j][k][l])
                    )
                = std::tie(
                    std::get<1>(comp[k][l][i][j]),
                    std::get<0>(comp[k][l][i][j]),
                    std::get<3>(comp[k][l][i][j]),
                    std::get<2>(comp[k][l][i][j])
                    )
                = std::make_tuple(
                    rl_comparison(image[i][j], image[k][l]),
                    lr_comparison(image[i][j], image[k][l]),
                    ud_comparison(image[i][j], image[k][l]),
                    du_comparison(image[i][j], image[k][l])
                );
            }
        }
    }
    return comp;
}


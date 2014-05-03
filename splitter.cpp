#include <algorithm>
#include <limits>
#include "splitter.hpp"

void recursive_operation(std::pair<int,int> const now, split_image_type const& split_image, std::vector<std::vector<std::pair<int,int>>>& output)
{
//#ifdef DEBUG
//    assert(output.size() == split_image.size());
//    for(int i=0; i<split_image.size(); ++i) assert(output[i].size() == split_image[i].size());
//#endif
//
//    // nowの上下左右に隣接するブロックを探して，それに再帰する
//    {
//        auto const target = std::make_pair(now.first + 1, now.second); // 右
//        bool const result = std::none_of(output.cbegin(), output.cend(),
//            [target](std::vector<std::pair<int,int>> const& inner)
//            {
//                return !std::none_of(inner.cbegin(), inner.cend(),
//                    [target](std::pair<int,int> const& p)
//                    {
//                        return p==target;
//                    });
//            });
//
//        if(result)
//        {
//            std::vector<std::vector<std::tuple<char,char,char>>>& now_image;
//
//            for(int i=0; i<split_image.size(); ++i) for(int j=0; j<split_image[0].size(); ++j)
//            {
//                if(output[i][j] == now)
//                {
//                    now_image = split_image[i][j];
//                    break;
//                }
//            }
//
//            std::vector<std::vector<int>> norm(
//                split_image.size(),
//                std::vector<int>(split_image[0].size(), 0)
//                );
//
//
//
//
//
//
//
//        }
//    }
//


    return;
}

question_data splitter::operator() (question_raw_data const& raw) const
{
    question_data formed = {
        raw.split_num,
        raw.selectable_num,
        raw.cost.first,
        raw.cost.second,
        std::vector<std::vector<int>>(raw.split_num.second, std::vector<int>(raw.split_num.first, INT_MAX) )
    };

    auto& block = formed.block;

    //
    // Sub Algorithm
    // 正しい位置に並べた時に左上から，1~nまでの番号をふり，それが今どこにあるのかという情報をblockに格納
    //

    auto const split_pixels = split_image(raw);



    // Sub Algorithm End

    return formed;
}

//nまでの和を求める関数
inline int splitter::n_sum(int n) const
{
    return n * (n+1) / 2;
}

//pixel比較
split_image_type splitter::split_image(question_raw_data const& raw) const
{
    int const parts_width = raw.size.first / raw.split_num.first;
    int const parts_height = raw.size.second / raw.split_num.second;

    std::vector<std::vector<std::vector<std::vector<std::tuple<uint8_t,uint8_t,uint8_t>>>>> split_pixels(
        raw.split_num.second,
        std::vector<std::vector<std::vector<std::tuple<uint8_t,uint8_t,uint8_t>>>>(
            raw.split_num.first,
            std::vector<std::vector<std::tuple<uint8_t,uint8_t,uint8_t>>>(
                parts_height,
                std::vector<std::tuple<uint8_t,uint8_t,uint8_t>>(
                    parts_width,
                    std::make_tuple(0,0,0)
                )
            )
        )
    );

    for(int i=0; i<raw.split_num.second; ++i) for(int j=0; j<raw.split_num.first; ++j)
    {
        int const splitting_pos = i * raw.split_num.first + j;
        for(int k=0; k<parts_height; ++k) for(int l=0; l<parts_width; ++l)
        {
            split_pixels[i][j][k][l] = raw.pixels[parts_height * i + k][parts_width * j + l];
        }
    }

    return split_pixels;
}

int splitter::pixel_comparison(pixel_type const& lhs, pixel_type const& rhs) const
{
    // TODO: Memo: えっと，これはノルムになってないよね？(commented by: godai_0519)
    int s = 0;
    s += std::abs(std::get<0>(lhs) - std::get<0>(rhs)); //r
    s += std::abs(std::get<1>(lhs) - std::get<1>(rhs)); //g
    s += std::abs(std::get<2>(lhs) - std::get<2>(rhs)); //b
    return s;
}

//一枚目の右端と二枚目の左端を見る関数, 一致が多いほど低いを返す
uint64_t splitter::rl_comparison(image_type const& lhs, image_type const& rhs) const
{
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs[i].back(), rhs[i].front());
    return s;
}

//一枚目の左端と二枚目の右端を見る関数, 一致が多いほど低いを返す
uint64_t splitter::lr_comparison(image_type const& lhs, image_type const& rhs) const
{
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs[i].front(), rhs[i].back());
    return s;
}

//一枚目の上端と二枚目の下端を見る関数, 一致が多いほど低いを返す
uint64_t splitter::ud_comparison(image_type const& lhs, image_type const& rhs) const
{
    auto const& lhs_top    = lhs.front();
    auto const& rhs_bottom = rhs.back();
        
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs_top[i], rhs_bottom[i]);
    return s;
}

//一枚目の下端と二枚目の上端を見る関数, 一致が多いほど低いを返す
uint64_t splitter::du_comparison(image_type const& lhs, image_type const& rhs) const
{
    auto const& lhs_bottom = lhs.back();
    auto const& rhs_top    = rhs.front();
        
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs_bottom[i], rhs_top[i]);
    return s;
}

compared_type splitter::image_comp(split_image_type const& image) const
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

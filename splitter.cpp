#include <climits>
#include "splitter.hpp"

// 気持ち悪いが，[i][j]の位置に分割された画像が入っている．更に[j][k]へのアクセスによって画素にアクセス
typedef std::vector<std::vector<std::vector<std::vector<std::tuple<char,char,char>>>>> split_image_type;

void recursive_operation(std::pair<int,int> const now, split_image_type const& split_image, std::vector<std::vector<int>>& output)
{
    //// nowの上下左右に隣接するブロックを探して，それに再帰する
    //{
    //    auto const target = std::make_pair(now.first + 1, now.second);
    //    for(
    //}


    return;
}

split_image_type split_image(question_raw_data const& raw)
{
    int const parts_width = raw.size.first / raw.split_num.first;
    int const parts_height = raw.size.second / raw.split_num.second;

    std::vector<std::vector<std::vector<std::vector<std::tuple<char,char,char>>>>> split_pixels(
        raw.split_num.second,
        std::vector<std::vector<std::vector<std::tuple<char,char,char>>>>(
            raw.split_num.first,
            std::vector<std::vector<std::tuple<char,char,char>>>(
                parts_height,
                std::vector<std::tuple<char,char,char>>(
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

question_data splitter::operator() (question_raw_data const& raw)
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

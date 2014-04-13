#include <climits>
#include "splitter.hpp"


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



    // Sub Algorithm End

    return formed;
}

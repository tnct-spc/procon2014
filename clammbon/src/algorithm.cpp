#include <algorithm>
#include "algorithm.hpp"

void algorithm::operator() (question_data const& data)
{
    //
    // Main Algorithm
    // 正しい順序に並べてみたり，データ送ったり．
    //
    // Ian Parberry "A Real-Time Algorithm for the (n^2-1)-Puzzle"
    //

    matrix = data.block;
}

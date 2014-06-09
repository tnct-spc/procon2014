#include "algorithm.hpp"

void algorithm::operator() (question_data const& data)
{
    //
    // Main Algorithm
    // 正しい順序に並べてみたり，データ送ったり．
    //
}

int algorithm::distance_between(point_type const& p0, point_type const& p1)
{
    // 2つの座標を受け取ってマンハッタン距離を返す
    return abs(p0.x - p1.x) + abs(p0.y - p1.y);
}

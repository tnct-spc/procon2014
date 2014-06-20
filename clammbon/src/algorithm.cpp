#include <algorithm>
#include "algorithm.hpp"

void algorithm::operator() (question_data const& data)
{
    //
    // Main Algorithm
    // 正しい順序に並べてみたり，データ送ったり．
    //

    matrix = data.block;
    direction_type<int>* directions = new direction_type<int>;
    directions->up = 0, directions->right = 1, directions->down = 2, directions->left = 3;
}

int algorithm::manhattan_distance(point_type& p0, point_type& p1)
{
    // 2つの座標を受け取ってマンハッタン距離を返す
    return abs(p0.x - p1.x) + abs(p0.y - p1.y);
}

int algorithm::distance_to_origial(point_type& point)
{
    // 1つの座標を受け取って, その座標に存在する断片画像の, 原座標までのマンハッタン距離を返す
    return manhattan_distance(point, matrix[point.y][point.x]);
}

bool algorithm::check_finish()
{
    // 終了判定
    bool finished = true;
    for (std::vector<point_type> row : matrix) {
        for (point_type point : row) {
            if (distance_to_origial(point) != 0) {
                finished = false;
            }
            break;
        }
        if (!finished) {
            break;
        }
    }

    return finished;
}

std::vector<point_type>* algorithm::gen_select_queue()
{
    // 選択キューを作る
    // とりあえずフラットにするだけ
    std::vector<point_type> *select_queue = new std::vector<point_type>;
    for (std::vector<point_type> row : matrix) {
        for (point_type point : row) {
            select_queue->push_back(point);
        }
    }
    return select_queue;
}

std::vector<point_type>* algorithm::gen_change_queue()
{
    // 交換キューを作る
    // 幅優先なので順番はどうでもいい

   // std::vector<point_type>* change_queue = new std::vector<point_type>;
}

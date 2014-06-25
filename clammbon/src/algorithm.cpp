#include <iostream>
#include <iomanip>
#include <algorithm>
#include "algorithm.hpp"

void algorithm::operator() (question_data const& data)
{
    //
    // Main Algorithm
    //
    // Ian Parberry
    // A Real-Time Algorithm for the (n^2-1)-Puzzle
    // http://local.cdn.cs50.net/2010/fall/psets/3/saml.pdf
    //
    // point_type を使って指しているものが座標なのか断片画像なのかわかりにくい (型の意味としては座標だが)
    //

    // 画像
    matrix = data.block;
    // 方向
    directions = direction_type<int>{0, 1, 2, 3};
    // 幅と高さ
    width = data.size.first;
    height = data.size.second;
    // ソート済み行及び列
    // この値の行及び列を含む内側部分を操作する
    sorted_row = 0;
    sorted_col = 0;
    // 移動に用いるタイルの原座標
    // 右下を選んでおけば間違いない
    mover_org = point_type{width - 1, height - 1};

    // GO
    solve();
}

point_type algorithm::current_point(point_type const& point)
{
    // point を原座標として持つタイルの現在の座標を返す
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (matrix[y][x] == point) {
                return point_type{x, y};
            }
        }
    }
}

void algorithm::solve()
{
    // Ian Parberry 氏のアルゴリズムを長方形に拡張したもの
    // とりあえず1回選択のみ

    for (;;) {
        // 3x3 の場合は Brute-Force
        if (height - sorted_row <= 3 && width - sorted_col <= 3) {
            brute_force();
            break;
        }

        // 貪欲法を適用
        greedy();

        // ソート済みマーク
        // ここで同時に縮めなければ長方形でも動くはず
        if (height - sorted_row > 3) {
            ++sorted_row;
        }
        if (width - sorted_col > 3) {
            ++sorted_col;
        }
    }
}

void algorithm::greedy()
{
    // 貪欲法で解く 一番要の部分

    // ターゲットをキューに入れる
    std::vector<point_type> target_queue;
    for (int i = sorted_col; i < width; i++) {
        target_queue.push_back(point_type{i, sorted_row});
    }
    for (int i = sorted_row + 1; i < height; i++) {
        target_queue.push_back(point_type{sorted_col, i});
    }

    // 全てのターゲットについて処理
    for (point_type target : target_queue) {
        // mover をターゲットの右へ移動させる
        move_from_to(current_point(mover_org), target + point_type{1, 0});

        // x, y のどちらかが一致するまで左上へ移動させる
        // 一致したら水平または垂直に移動
        for (;;) {
            print();
            if (current_point(mover_org).x < mover_org.x) {
                // 上へ移動
                move_direction(current_point(mover_org), directions.up);
                move_direction(current_point(mover_org), directions.left);
                move_direction(current_point(mover_org), directions.down);
            }
            if (current_point(mover_org).y < mover_org.y) {
                // 左へ移動
                move_direction(current_point(mover_org), directions.left);
                move_direction(current_point(mover_org), directions.up);
                move_direction(current_point(mover_org), directions.right);
            }

            // 原座標に戻ったら break
            if (current_point(mover_org) == mover_org) {
                break;
            }
        }
    }
}

void algorithm::brute_force()
{
    // Brute-Force Algorithm
    // とりあえず選択画像を変更しない幅優先探索で

    // TODO: これからやる
}

void algorithm::move_direction(point_type const& target, int const& direction)
{
    // target に存在するタイルを指定された方向へ移動する
    if (direction == directions.up) {
        std::swap(matrix[target.y][target.x], matrix[target.y - 1][target.x]);
    } else if (direction == directions.right) {
        std::swap(matrix[target.y][target.x], matrix[target.y][target.x + 1]);
    } else if (direction == directions.down) {
        std::swap(matrix[target.y][target.x], matrix[target.y + 1][target.x]);
    } else if (direction == directions.left) {
        std::swap(matrix[target.y][target.x], matrix[target.y][target.x - 1]);
    }
}

void algorithm::move_from_to(point_type from, point_type const& to)
{
    // from に存在するタイルを to まで移動させる
    // 先に縦に移動することをここでは保証する

    // Y座標が異る場合
    if (from.y != to.y) {
        // 縦に移動する
        for (;;) {
            print();
            if (from.y > to.y) {
                move_direction(from, directions.up);
                --from.y;
            } else if (from.y < to.y) {
                move_direction(from, directions.down);
                ++from.y;
            } else {
                break;
            }
        }
    }

    // X座標が異る場合
    if (from.x != to.x) {
        // 横に移動する
        for (;;) {
            print();
            if (from.x > to.x) {
                move_direction(from, directions.left);
                --from.x;
            } else if (from.x < to.x) {
                move_direction(from, directions.right);
                ++from.x;
            } else {
                break;
            }
        }
    }
}

int algorithm::point_to_num(point_type const& point)
{
    // point_type を座標番号に変換する
    return point.x * 16 + point.y;
}

void algorithm::print()
{
    // 具合をいい感じに表示
    std::cout << "selecting : " << std::uppercase << std::hex << point_to_num(mover_org) << " at (" << current_point(mover_org).x << ", " << current_point(mover_org).y << ")" << std::endl;
    for (std::vector<point_type> row : matrix) {
        for (point_type tile : row) {
            std::cout << std::setw(3) << std::uppercase << std::hex << point_to_num(tile);
        }
        std::cout << std::endl;
    }
    std::cin.ignore();
}

/* 
// 検証用main
// g++ -std=c++11 -I../include algorithm.cpp ; ./a.out
int main(int argc, char* argv[])
{
    std::vector<std::vector<point_type>> matrix;
    matrix.push_back(std::vector<point_type>{point_type{3, 1}, point_type{7, 3}, point_type{2, 4}, point_type{5, 7}, point_type{4, 7}, point_type{7, 2}, point_type{5, 5}, point_type{3, 5}});
    matrix.push_back(std::vector<point_type>{point_type{0, 1}, point_type{6, 7}, point_type{4, 4}, point_type{3, 2}, point_type{4, 3}, point_type{1, 2}, point_type{2, 3}, point_type{1, 3}});
    matrix.push_back(std::vector<point_type>{point_type{7, 1}, point_type{0, 6}, point_type{4, 0}, point_type{3, 7}, point_type{1, 6}, point_type{7, 6}, point_type{3, 4}, point_type{2, 5}});
    matrix.push_back(std::vector<point_type>{point_type{2, 2}, point_type{4, 6}, point_type{0, 3}, point_type{2, 6}, point_type{2, 1}, point_type{1, 5}, point_type{5, 2}, point_type{3, 6}});
    matrix.push_back(std::vector<point_type>{point_type{0, 2}, point_type{2, 0}, point_type{0, 4}, point_type{4, 1}, point_type{5, 0}, point_type{6, 0}, point_type{5, 6}, point_type{1, 0}});
    matrix.push_back(std::vector<point_type>{point_type{6, 3}, point_type{5, 3}, point_type{0, 5}, point_type{7, 5}, point_type{2, 7}, point_type{7, 4}, point_type{4, 2}, point_type{1, 7}});
    matrix.push_back(std::vector<point_type>{point_type{3, 3}, point_type{6, 2}, point_type{7, 0}, point_type{4, 5}, point_type{5, 1}, point_type{0, 0}, point_type{1, 4}, point_type{3, 0}});
    matrix.push_back(std::vector<point_type>{point_type{7, 7}, point_type{6, 1}, point_type{0, 7}, point_type{1, 1}, point_type{6, 4}, point_type{5, 4}, point_type{6, 5}, point_type{6, 6}});

    algorithm a = algorithm();
    a(question_data(std::pair<int, int>(5, 5), 16, 1, 1, matrix));
}
*/

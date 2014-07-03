#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <cstdint>
#include <tuple>
#include <boost/format.hpp>
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
    // 幅と高さ
    width = data.size.first;
    height = data.size.second;
    // ソート済み行及び列
    // この値の行及び列を含む内側部分を操作する
    sorted_row = 0;
    sorted_col = 0;
    // 移動に用いる断片画像の原座標
    // 右下を選んでおけば間違いない
    mover = point_type{width - 1, height - 1};

    // GO
    print();
    solve();
}

point_type algorithm::current_point(point_type const& point)
{
    // point を原座標として持つ断片画像の現在の座標を返す
    for (int y = 0; y < height; ++y) {
        auto p = std::find(matrix[y].begin(), matrix[y].end(), point);
        if (p == matrix[y].end()) {
            continue;
        }
        return point_type{static_cast<int>(std::distance(matrix[y].begin(), p)), y};
    }
    throw std::runtime_error(std::string("No Tile ") + std::to_string(point.x) + std::string(", ") +  std::to_string(point.y));
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
    // ここで target の中身は原座標である
    // queue なのに vector とはこれいかに
    std::vector<point_type> target_queue;
    for (int i = sorted_col; i < width; i++) {
        target_queue.push_back(point_type{i, sorted_row});
    }
    for (int i = sorted_row + 1; i < height; i++) {
        target_queue.push_back(point_type{sorted_col, i});
    }

    // 原座標と現在の座標との差
    point_type point_diff;
    // どっちを周るか
    TurnSide turnside;
    // カウンタ
    int i;
    for (point_type target : target_queue) {
        // まず斜めに移動
        point_diff = target - current_point(target);

        // TODO: 水平垂直のどちらを先に動かすかの判断も必要では
        i = 0;
        while (point_diff.x < 0 && point_diff.y < 0) {
            // 原座標に対して右下にあるので左上へ移動
            if (i % 2 == 0) {
                move_target_direction(current_point(target), Direction::Up, TurnSide::DownerRight);
            } else {
                move_target_direction(current_point(target), Direction::Left, TurnSide::DownerRight);
            }
            ++i;
            point_diff = target - current_point(target);
        }
        i = 0;
        while (point_diff.x < 0 && point_diff.y > 0) {
            // 原座標に対して右上にあるので左下へ移動
            if (i % 2 == 0) {
                move_target_direction(current_point(target), Direction::Down, TurnSide::UpperRight);
            } else {
                move_target_direction(current_point(target), Direction::Left, TurnSide::UpperRight);
            }
            ++i;
            point_diff = target - current_point(target);
        }
        i = 0;
        while (point_diff.x > 0 && point_diff.y < 0) {
            // 原座標に対して左下にあるので右上へ移動
            if (i % 2 == 0) {
                move_target_direction(current_point(target), Direction::Up, TurnSide::DownerLeft);
            } else {
                move_target_direction(current_point(target), Direction::Right, TurnSide::DownerLeft);
            }
            ++i;
            point_diff = target - current_point(target);
        }
        i = 0;
        while (point_diff.x > 0 && point_diff.y > 0) {
            // 原座標に対して左上にあるので右下へ移動
            if (i % 2 == 0) {
                move_target_direction(current_point(target), Direction::Down, TurnSide::UpperLeft);
            } else {
                move_target_direction(current_point(target), Direction::Right, TurnSide::UpperLeft);
            }
            ++i;
            point_diff = target - current_point(target);
        }

        // ここまでで x または y 座標が揃っていることになるので真横か真上への移動を行う

        // 周る方向を決める
        if (target.x < width / 2) {
            // ターゲットが上側なので下側を周る
            if (target.y < width / 2) {
                // ターゲットが左側なので右側を周る
                turnside = TurnSide::DownerRight;
            } else {
                // ターゲットが右側なので左側を周る
                turnside = TurnSide::DownerLeft;
            }
        } else {
            // ターゲットが下側なので上側を周る
            if (target.y < width / 2) {
                // ターゲットが左側なので右側を周る
                turnside = TurnSide::UpperRight;
            } else {
                // ターゲットが右側なので左側を周る
                turnside = TurnSide::UpperLeft;
            }
        }

        point_diff = target - current_point(target);
        if (point_diff.x != 0) {
            // 横に移動する場合
            while (point_diff.x < 0) {
                // 原座標より右にいるので左へ移動
                move_target_direction(current_point(target), Direction::Left, turnside);
                point_diff = target - current_point(target);
            }
            while (point_diff.x > 0) {
                // 原座標より左にいるので右へ移動
                move_target_direction(current_point(target), Direction::Right, turnside);
                point_diff = target - current_point(target);
            }
        } else if (point_diff.y != 0) {
            // 縦に移動する場合
            while (point_diff.y > 0) {
                // 原座標より上にいるので下へ移動
                move_target_direction(current_point(target), Direction::Down, turnside);
                point_diff = target - current_point(target);
            }
            while (point_diff.y < 0) {
                // 原座標より下にいるので上へ移動
                move_target_direction(current_point(target), Direction::Up, turnside);
                point_diff = target - current_point(target);
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

void algorithm::move_direction(point_type const& target, Direction const& direction)
{
    // target に存在する断片画像を指定された方向へ移動する
    print();
    assert(0 <= target.x && target.x < width && 0 <= target.y && target.y < height);
    if (direction == Direction::Up) {
        assert(target.y > 0);
        std::swap(matrix[target.y][target.x], matrix[target.y - 1][target.x]);
    } else if (direction == Direction::Right) {
        assert(target.x < width - 1);
        std::swap(matrix[target.y][target.x], matrix[target.y][target.x + 1]);
    } else if (direction == Direction::Down) {
        assert(target.y < height - 1);
        std::swap(matrix[target.y][target.x], matrix[target.y + 1][target.x]);
    } else if (direction == Direction::Left) {
        assert(target.x > 0);
        std::swap(matrix[target.y][target.x], matrix[target.y][target.x - 1]);
    }
}

void algorithm::move_from_to(point_type from, point_type const& to)
{
    // from に存在する断片画像を to まで移動させる

    // 斜め方向への移動はとりあえずダメってことで
    assert(from.x == to.x || from.y == to.y);

    // Y座標が異なる場合に縦に移動する
    while (from.y != to.y) {
        if (from.y > to.y) {
            move_direction(from, Direction::Up);
            --from.y;
        } else {
            move_direction(from, Direction::Down);
            ++from.y;
        }
    }

    // X座標が異なる場合に横に移動する
    while (from.x != to.x) {
        if (from.x > to.x) {
            move_direction(from, Direction::Left);
            --from.x;
        } else {
            move_direction(from, Direction::Right);
            ++from.x;
        }
    }
}

void algorithm::move_target_direction(point_type const& target, Direction const& direction, TurnSide const& turnside)
{
    // mover の操作によって target に存在する断片画像を指定の方向へ移動させる.
    // NOTE: 無条件に縦に先に移動する仕様だがこれでいいのか？

    // mover を操作のための初期位置に移動させる.
    // 例えば, target を上へ移動させる場合, mover は target の1つ上にいる必要がある.

    // mover の現在の座標
    point_type mover_cur = current_point(mover);
    // 通過点3箇所
    std::tuple<point_type, point_type, point_type> pass_points;

    if (direction == Direction::Up) {
        // 上方向に移動させたい
        if (mover_cur.x == target.x && mover_cur.y > target.y) {
            // mover が target の真下にいる場合
            if (turnside == TurnSide::UpperRight || turnside == TurnSide::DownerRight) {
                // 右側を周る場合
                std::get<0>(pass_points) = mover_cur + point_type{1, 0};
            } else {
                // 左側を周る場合
                std::get<0>(pass_points) = mover_cur + point_type{-1, 0};
            }
        } else {
            std::get<0>(pass_points) = mover_cur;
        }
        std::get<1>(pass_points) = point_type{std::get<0>(pass_points).x, target.y - 1};
        std::get<2>(pass_points) = point_type{target.x, target.y - 1};
    } else if (direction == Direction::Right) {
        // 右方向に移動させたい
        if (mover_cur.y == target.y && mover_cur.x < target.x) {
            // mover が target の真左にいる場合
            if (turnside == TurnSide::DownerRight || turnside == TurnSide::DownerLeft) {
                // 下側を周る場合
                std::get<0>(pass_points) = mover_cur + point_type{0, 1};
            } else {
                // 上側を周る場合
                std::get<0>(pass_points) = mover_cur + point_type{0, -1};
            }
        } else {
            std::get<0>(pass_points) = mover_cur;
        }
        std::get<1>(pass_points) = point_type{target.x + 1, std::get<0>(pass_points).y};
        std::get<2>(pass_points) = point_type{target.x + 1, target.y};
    } else if (direction == Direction::Down) {
        // 下方向に移動させたい
        if (mover_cur.x == target.x && mover_cur.y < target.y) {
            // mover が target の真上にいる場合
            if (turnside == TurnSide::UpperRight || turnside == TurnSide::DownerRight) {
                // 右側を周る場合
                std::get<0>(pass_points) = mover_cur + point_type{1, 0};
            } else {
                // 左側を周る場合
                std::get<0>(pass_points) = mover_cur + point_type{-1, 0};
            }
        } else {
            std::get<0>(pass_points) = mover_cur;
        }
        std::get<1>(pass_points) = point_type{std::get<0>(pass_points).x, target.y + 1};
        std::get<2>(pass_points) = point_type{target.x, target.y + 1};
    } else if (direction == Direction::Left) {
        // 左方向に移動させたい
        if (mover_cur.y == target.y && mover_cur.x > target.x) {
            // mover が target の右側にいる場合
            if (turnside == TurnSide::DownerRight || turnside == TurnSide::DownerLeft) {
                // 下側を周る場合
                std::get<0>(pass_points) = mover_cur + point_type{0, 1};
            } else {
                // 上側を周る場合
                std::get<0>(pass_points) = mover_cur + point_type{0, -1};
            }
        } else {
            std::get<0>(pass_points) = mover_cur;
        }
        std::get<1>(pass_points) = point_type{target.x - 1, std::get<0>(pass_points).y};
        std::get<2>(pass_points) = point_type{target.x - 1, target.y};
    }

    move_from_to(mover_cur, std::get<0>(pass_points));
    move_from_to(std::get<0>(pass_points), std::get<1>(pass_points));
    move_from_to(std::get<1>(pass_points), std::get<2>(pass_points));

    // mover を移動
    move_direction(current_point(mover), inverse_direction(direction));
}

Direction algorithm::inverse_direction(Direction const& direction) {
    // 逆向きの方向を返す
    if (direction == Direction::Up) {
        return Direction::Down;
    } else if (direction == Direction::Down) {
        return Direction::Up;
    } else if (direction == Direction::Right) {
        return Direction::Left;
    } else if (direction == Direction::Left) {
        return Direction::Right;
    }
}

uint16_t algorithm::point_to_num(point_type const& point)
{
    // point_type を座標番号に変換する
    return point.x * 16 + point.y;
}

void algorithm::print()
{
    // 具合をいい感じに表示
    std::cout << boost::format("selecting %1$02x at (%2%)") % point_to_num(mover) % current_point(mover) << std::endl;
    for (std::vector<point_type> row : matrix) {
        for (point_type tile : row) {
            std::cout << std::setfill('0') << std::setw(2) << std::hex << point_to_num(tile) << std::dec << " ";
        }
        std::cout << std::endl;
    }
    std::cin.ignore();
}

// 検証用main
// これどうやってテストすれば？
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
    a(question_data(std::pair<int, int>(8, 8), 16, 1, 1, matrix));
}

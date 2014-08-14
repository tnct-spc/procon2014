#define algorithm_debug
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cassert>
#include "algorithm.hpp"
//#include "network.hpp"

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

    // コストとレート
    cost_select = data.cost_select;
    cost_change = data.cost_change;

    // ソート済み行及び列
    // この値の行及び列を含む内側部分を操作する
    sorted_row = 0;
    sorted_col = 0;

    // ソート済みの断片画像リスト
    sorted_points = std::vector<point_type>();

    // 移動に用いる断片画像の原座標
    // 右下を選んでおけば間違いない
    mover = point_type{width - 1, height - 1};

    answer.list.push_back(answer_line{mover, std::vector<HVDirection>{}});

    // GO
#ifdef algorithm_debug
    print();
#endif
    solve();
}

const point_type algorithm::current_point(point_type const& point) const
{
    // point を原座標として持つ断片画像の現在の座標を返す
    for (int y = 0; y < height; ++y) {
        auto p = std::find(matrix[y].begin(), matrix[y].end(), point);
        if (p == matrix[y].end()) {
            continue;
        }
        return point_type{static_cast<int>(std::distance(matrix[y].begin(), p)), y};
    }
    throw std::runtime_error("No Tile " + point.str());
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
    DiagonalDirection turnside;
    // カウンタ
    int i;
    // 仮のターゲット座標, これは原座標ではなく実際の座標で固定
    point_type target;
    for (point_type target_org : target_queue) {
        // 端の部分の処理
        if (target_org.x == width - 2 || target_org.y == height - 1) {
            // ターゲットが右から2番目の断片画像のとき
            // ターゲットが下から1番目の断片画像のとき
            target = target_org.right();
        } else if (target_org.x == width - 1 || target_org.y == height - 2) {
            // ターゲットが右から1番目の断片画像のとき
            // ターゲットが下から2番目の断片画像のとき
            target = target_org.down();
        } else {
            target = target_org;
        }

        // まず斜めに移動
        point_diff = target - current_point(target_org);

        // TODO: 水平垂直のどちらを先に動かすかの判断も必要では

        // 特に理由がなければ縦方向に先に動くが、移動先がソート済みの場合は横に先に動く
        // これ4回もコピペすんの
        i = 0;
        if (std::find(sorted_points.begin(), sorted_points.end(), current_point(target_org).up()) != sorted_points.end()) {
            ++i;
        }
        while (point_diff.x < 0 && point_diff.y < 0) {
            // 原座標に対して右下にあるので左上へ移動
            if (i % 2 == 0) {
                move_target_direction(current_point(target_org), HVDirection::Up, DiagonalDirection::DownerRight);
            } else {
                move_target_direction(current_point(target_org), HVDirection::Left, DiagonalDirection::DownerRight);
            }
            ++i;
            point_diff = target - current_point(target_org);
        }

        i = 0;
        if (std::find(sorted_points.begin(), sorted_points.end(), current_point(target_org).down()) != sorted_points.end()) {
            ++i;
        }
        while (point_diff.x < 0 && point_diff.y > 0) {
            // 原座標に対して右上にあるので左下へ移動
            if (i % 2 == 0) {
                move_target_direction(current_point(target_org), HVDirection::Down, DiagonalDirection::UpperRight);
            } else {
                move_target_direction(current_point(target_org), HVDirection::Left, DiagonalDirection::UpperRight);
            }
            ++i;
            point_diff = target - current_point(target_org);
        }

        i = 0;
        if (std::find(sorted_points.begin(), sorted_points.end(), current_point(target_org).up()) != sorted_points.end()) {
            ++i;
        }
        while (point_diff.x > 0 && point_diff.y < 0) {
            // 原座標に対して左下にあるので右上へ移動
            if (i % 2 == 0) {
                move_target_direction(current_point(target_org), HVDirection::Up, DiagonalDirection::DownerLeft);
            } else {
                move_target_direction(current_point(target_org), HVDirection::Right, DiagonalDirection::DownerLeft);
            }
            ++i;
            point_diff = target - current_point(target_org);
        }

        i = 0;
        if (std::find(sorted_points.begin(), sorted_points.end(), current_point(target_org).down()) != sorted_points.end()) {
            ++i;
        }
        while (point_diff.x > 0 && point_diff.y > 0) {
            // 原座標に対して左上にあるので右下へ移動
            if (i % 2 == 0) {
                move_target_direction(current_point(target_org), HVDirection::Down, DiagonalDirection::UpperLeft);
            } else {
                move_target_direction(current_point(target_org), HVDirection::Right, DiagonalDirection::UpperLeft);
            }
            ++i;
            point_diff = target - current_point(target_org);
        }

        // ここまでで x または y 座標が揃っていることになるので真横か真上への移動を行う

        // 周る方向を決める
        if (target.x == width - 1) {
            // ターゲットが最右列なので左を周る
            if (target.y == height - 1) {
                // ターゲットが最下行なので上を周る
                turnside = DiagonalDirection::UpperLeft;
            } else {
                turnside = DiagonalDirection::DownerLeft;
            }
        } else {
            if (target.y == height - 1) {
                // ターゲットが最下行なので上を周る
                turnside = DiagonalDirection::UpperRight;
            } else {
                turnside = DiagonalDirection::DownerRight;
            }
        }

        point_diff = target - current_point(target_org);
        if (point_diff.x != 0) {
            // 横に移動する場合
            while (point_diff.x < 0) {
                // 原座標より右にいるので左へ移動
                move_target_direction(current_point(target_org), HVDirection::Left, turnside);
                point_diff = target - current_point(target_org);
            }
            while (point_diff.x > 0) {
                // 原座標より左にいるので右へ移動
                move_target_direction(current_point(target_org), HVDirection::Right, turnside);
                point_diff = target - current_point(target_org);
            }
        } else if (point_diff.y != 0) {
            // 縦に移動する場合
            while (point_diff.y > 0) {
                // 原座標より上にいるので下へ移動
                move_target_direction(current_point(target_org), HVDirection::Down, turnside);
                point_diff = target - current_point(target_org);
            }
            while (point_diff.y < 0) {
                // 原座標より下にいるので上へ移動
                move_target_direction(current_point(target_org), HVDirection::Up, turnside);
                point_diff = target - current_point(target_org);
            }
        }

        // 端の部分の処理
        if (target_org.x == width - 1) {
            // ターゲットの真の原座標が右端の場合
            if (current_point(mover) == target.down()) {
                // mover が仮の原座標の直下にいる場合
                move_direction(current_point(mover), HVDirection::Left);
                move_direction(current_point(mover), HVDirection::Up);
            }
            move_direction(current_point(mover), HVDirection::Up);
            move_direction(current_point(mover), HVDirection::Right);
            move_direction(current_point(mover), HVDirection::Down);
            // ソート済みマーク
            sorted_points.push_back(target_org.left());
            sorted_points.push_back(target_org);
        } else if (target_org.y == height - 1) {
            // ターゲットの真の原座標が下端の場合
            if (current_point(mover) == target.right()) {
                // mover が仮の原座標の直右にいる場合
                move_direction(current_point(mover), HVDirection::Up);
                move_direction(current_point(mover), HVDirection::Left);
            }
            move_direction(current_point(mover), HVDirection::Left);
            move_direction(current_point(mover), HVDirection::Down);
            move_direction(current_point(mover), HVDirection::Right);
            // ソート済みマーク
            sorted_points.push_back(target_org.up());
            sorted_points.push_back(target_org);
        } else if (target_org.x < width - 2 && target_org.y < width - 2) {
            // ソート済みマーク
            sorted_points.push_back(target_org);
        }
    }
}

void algorithm::brute_force()
{
    // Brute-Force Algorithm
    // とりあえず選択画像を変更しない幅優先探索で

    // TODO: これからやる
}

void algorithm::move_direction(point_type const& target, HVDirection const& direction)
{
    // target に存在する断片画像を指定された方向へ移動する
#ifdef algorithm_debug
    print();
#endif
    assert(sorted_col <= target.x && target.x < width && sorted_row <= target.y && target.y < height);
    if (direction == HVDirection::Up) {
        assert(target.y > sorted_row);
        std::swap(matrix[target.y][target.x], matrix[target.y - 1][target.x]);
    } else if (direction == HVDirection::Right) {
        assert(target.x < width - 1);
        std::swap(matrix[target.y][target.x], matrix[target.y][target.x + 1]);
    } else if (direction == HVDirection::Down) {
        assert(target.y < height - 1);
        std::swap(matrix[target.y][target.x], matrix[target.y + 1][target.x]);
    } else if (direction == HVDirection::Left) {
        assert(target.x > sorted_col);
        std::swap(matrix[target.y][target.x], matrix[target.y][target.x - 1]);
    }

    answer.list.back().change_list.push_back(direction);
}

void algorithm::move_from_to(point_type const& from, point_type const& to)
{
    // from に存在する断片画像を to まで移動させる

    assert(from.x == to.x || from.y == to.y);

    point_type new_from = matrix[from.y][from.x];

    while (current_point(new_from).y > to.y) {
        move_direction(current_point(new_from), HVDirection::Up);
    }
    while (current_point(new_from).y < to.y) {
        move_direction(current_point(new_from), HVDirection::Down);
    }
    while (current_point(new_from).x > to.x) {
        move_direction(current_point(new_from), HVDirection::Left);
    }
    while (current_point(new_from).x < to.x) {
        move_direction(current_point(new_from), HVDirection::Right);
    }
}

void algorithm::move_target_direction(point_type const& target, HVDirection const& direction, DiagonalDirection const& turnside)
{
    // mover の操作によって target に存在する断片画像を指定の方向へ移動させる.
    // NOTE: 無条件に縦に先に移動する仕様だがこれでいいのか？

    // mover を操作のための初期位置に移動させる.
    // 例えば, target を上へ移動させる場合, mover は target の1つ上にいる必要がある.

    // mover の現在の座標
    point_type mover_cur = current_point(mover);
    // 通過点4箇所
    std::tuple<point_type, point_type, point_type, point_type> pass_points;

    if (direction == HVDirection::Up) {
        // 上方向に移動させたい
        if (mover_cur.x == target.x && mover_cur.y > target.y) {
            // mover が target の真下にいる場合
            if (turnside == DiagonalDirection::UpperRight || turnside == DiagonalDirection::DownerRight) {
                // 右側を周る場合
                std::get<0>(pass_points) = mover_cur.right();
            } else {
                // 左側を周る場合
                std::get<0>(pass_points) = mover_cur.left();
            }
        } else {
            std::get<0>(pass_points) = mover_cur;
        }
        std::get<1>(pass_points) = point_type{std::get<0>(pass_points).x, target.y - 1};
        std::get<2>(pass_points) = target.up();
    } else if (direction == HVDirection::Right) {
        // 右方向に移動させたい
        if (mover_cur.y == target.y && mover_cur.x < target.x) {
            // mover が target の真左にいる場合
            if (turnside == DiagonalDirection::DownerRight || turnside == DiagonalDirection::DownerLeft) {
                // 下側を周る場合
                std::get<0>(pass_points) = mover_cur.down();
            } else {
                // 上側を周る場合
                std::get<0>(pass_points) = mover_cur.up();
            }
        } else {
            std::get<0>(pass_points) = mover_cur;
        }
        std::get<1>(pass_points) = point_type{target.x + 1, std::get<0>(pass_points).y};
        std::get<2>(pass_points) = target.right();
    } else if (direction == HVDirection::Down) {
        // 下方向に移動させたい
        if (mover_cur.x == target.x && mover_cur.y < target.y) {
            // mover が target の真上にいる場合
            if (turnside == DiagonalDirection::UpperRight || turnside == DiagonalDirection::DownerRight) {
                // 右側を周る場合
                std::get<0>(pass_points) = mover_cur.right();
            } else {
                // 左側を周る場合
                std::get<0>(pass_points) = mover_cur.left();
            }
        } else {
            std::get<0>(pass_points) = mover_cur;
        }
        std::get<1>(pass_points) = point_type{std::get<0>(pass_points).x, target.y + 1};
        std::get<2>(pass_points) = target.down();
    } else if (direction == HVDirection::Left) {
        // 左方向に移動させたい
        if (mover_cur.y == target.y && mover_cur.x > target.x) {
            // mover が target の右側にいる場合
            if (turnside == DiagonalDirection::DownerRight || turnside == DiagonalDirection::DownerLeft) {
                // 下側を周る場合
                std::get<0>(pass_points) = mover_cur.down();
            } else {
                // 上側を周る場合
                std::get<0>(pass_points) = mover_cur.up();
            }
        } else {
            std::get<0>(pass_points) = mover_cur;
        }
        std::get<1>(pass_points) = point_type{target.x - 1, std::get<0>(pass_points).y};
        std::get<2>(pass_points) = target.left();
    }

    move_from_to(mover_cur, std::get<0>(pass_points));
    move_from_to(std::get<0>(pass_points), std::get<1>(pass_points));
    move_from_to(std::get<1>(pass_points), std::get<2>(pass_points));

    // mover を移動
    move_direction(current_point(mover), inverse_direction(direction));
}

HVDirection algorithm::inverse_direction(HVDirection const& direction) const
{
    // 逆向きの方向を返す
    if (direction == HVDirection::Up) {
        return HVDirection::Down;
    } else if (direction == HVDirection::Down) {
        return HVDirection::Up;
    } else if (direction == HVDirection::Right) {
        return HVDirection::Left;
    } else if (direction == HVDirection::Left) {
        return HVDirection::Right;
    }
}

inline void algorithm::print() const
{
    // 具合をいい感じに表示
    std::cout << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << std::endl;
    for (std::vector<point_type> row : matrix) {
        for (point_type tile : row) {
            std::cout << boost::format("%1$02X ") % tile.num();
        }
        std::cout << std::endl;
    }
    int score_select = answer.list.size() * cost_select;
    int score_change = 0;
    for (auto step : answer.list) {
        score_change += step.change_list.size() * cost_change;
    }
    std::cout << std::endl;
    std::cout << boost::format("score = %1% (select = %2%, change = %3%)") % (score_select + score_change) % score_select % score_change << std::endl;
    std::cout << std::endl;
    std::cout << "answer:" << std::endl;
    std::cout << answer.str();
    std::cin.ignore();
}

#ifdef algorithm_debug
// 検証用main
// これどうやってテストすれば？
// g++ -std=c++11 -I../include algorithm.cpp
int main(int argc, char* argv[])
{
    const auto matrix = std::vector<std::vector<point_type>>{
        std::vector<point_type>{point_type{3, 1}, point_type{7, 3}, point_type{2, 4}, point_type{5, 7}, point_type{4, 7}, point_type{7, 2}, point_type{5, 5}, point_type{3, 5}},
        std::vector<point_type>{point_type{0, 1}, point_type{6, 7}, point_type{4, 4}, point_type{3, 2}, point_type{4, 3}, point_type{1, 2}, point_type{2, 3}, point_type{1, 3}},
        std::vector<point_type>{point_type{7, 1}, point_type{0, 6}, point_type{4, 0}, point_type{3, 7}, point_type{1, 6}, point_type{7, 6}, point_type{3, 4}, point_type{2, 5}},
        std::vector<point_type>{point_type{2, 2}, point_type{4, 6}, point_type{0, 3}, point_type{2, 6}, point_type{2, 1}, point_type{1, 5}, point_type{5, 2}, point_type{3, 6}},
        std::vector<point_type>{point_type{0, 2}, point_type{2, 0}, point_type{0, 4}, point_type{4, 1}, point_type{5, 0}, point_type{6, 0}, point_type{5, 6}, point_type{1, 0}},
        std::vector<point_type>{point_type{6, 3}, point_type{5, 3}, point_type{0, 5}, point_type{7, 5}, point_type{2, 7}, point_type{7, 4}, point_type{4, 2}, point_type{1, 7}},
        std::vector<point_type>{point_type{3, 3}, point_type{6, 2}, point_type{7, 0}, point_type{4, 5}, point_type{5, 1}, point_type{0, 0}, point_type{1, 4}, point_type{3, 0}},
        std::vector<point_type>{point_type{7, 7}, point_type{6, 1}, point_type{0, 7}, point_type{1, 1}, point_type{6, 4}, point_type{5, 4}, point_type{6, 5}, point_type{6, 6}}
    };
    const auto size = std::pair<int, int>(matrix[0].size(), matrix.size());
    constexpr int selectable = 16;
    constexpr int cost_select = 10;
    constexpr int cost_change = 10;
    const auto qdata = question_data(size, selectable, cost_select, cost_change, matrix);
    algorithm()(qdata);
}
#endif

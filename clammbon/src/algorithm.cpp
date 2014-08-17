#define algorithm_debug
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <string>
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

    // ソート済み断片画像
    std::vector<point_type> sorted_points;

    // 移動に用いる断片画像の原座標
    // 右下を選んでおけば間違いない
    selecting = point_type{width - 1, height - 1};

    answer.list.push_back(answer_line{selecting, std::vector<HVDirection>()});

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
    // 中身は原座標
    std::vector<point_type> target_queue;
    for (int i = sorted_col; i < width; i++) {
        target_queue.push_back(point_type{i, sorted_row});
    }
    for (int i = sorted_row + 1; i < height; i++) {
        target_queue.push_back(point_type{sorted_col, i});
    }

    // カウンタ
    int i;

    // 仮のターゲット座標, これは原座標ではなく実際の座標
    point_type waypoint;

    for (point_type target : target_queue) {
        // 端の部分の処理
        if (target.x == width - 2 || target.y == height - 1) {
            // ターゲットが右から2番目の断片画像のとき
            // ターゲットが下から1番目の断片画像のとき
            waypoint = target.right();
        } else if (target.x == width - 1 || target.y == height - 2) {
            // ターゲットが右から1番目の断片画像のとき
            // ターゲットが下から2番目の断片画像のとき
            waypoint = target.down();
        } else {
            waypoint = target;
        }

        // 斜めに移動
        if (current_point(target).direction(waypoint) == AllDirection::UpperRight) {
            i = is_sorted(current_point(target).up()) ? 1 : 0;
            do {
                if (i % 2 == 0) {
                    move_target(target, HVDirection::Up);
                } else {
                    move_target(target, HVDirection::Right);
                }
                ++i;
            } while (current_point(target).direction(waypoint) == AllDirection::UpperRight);
        } else if (current_point(target).direction(waypoint) == AllDirection::DownerRight) {
            i = is_sorted(current_point(target).up()) ? 1 : 0;
            do {
                if (i % 2 == 0) {
                    move_target(target, HVDirection::Down);
                } else {
                    move_target(target, HVDirection::Right);
                }
                ++i;
            } while (current_point(target).direction(waypoint) == AllDirection::DownerRight);
        } else if (current_point(target).direction(waypoint) == AllDirection::DownerLeft) {
            i = is_sorted(current_point(target).up()) ? 1 : 0;
            do {
                if (i % 2 == 0) {
                    move_target(target, HVDirection::Down);
                } else {
                    move_target(target, HVDirection::Left);
                }
                ++i;
            } while (current_point(target).direction(waypoint) == AllDirection::DownerLeft);
        } else if (current_point(target).direction(waypoint) == AllDirection::UpperLeft) {
            i = is_sorted(current_point(target).up()) ? 1 : 0;
            do {
                if (i % 2 == 0) {
                    move_target(target, HVDirection::Up);
                } else {
                    move_target(target, HVDirection::Left);
                }
                ++i;
            } while (current_point(target).direction(waypoint) == AllDirection::UpperLeft);
        } else {
            //throw
        }

        // ここまでで x または y 座標が揃っていることになるので, 真横か真上への移動を行う

        if (current_point(target).direction(waypoint) == AllDirection::Up) {
            do {
                move_target(target, HVDirection::Up);
            } while (current_point(target).direction(waypoint) == AllDirection::Up);
        } else if (current_point(target).direction(waypoint) == AllDirection::Right) {
            do {
                move_target(target, HVDirection::Right);
            } while (current_point(target).direction(waypoint) == AllDirection::Right);
        } else if (current_point(target).direction(waypoint) == AllDirection::Down) {
            do {
                move_target(target, HVDirection::Down);
            } while (current_point(target).direction(waypoint) == AllDirection::Down);
        } else if (current_point(target).direction(waypoint) == AllDirection::Left) {
            do {
                move_target(target, HVDirection::Left);
            } while (current_point(target).direction(waypoint) == AllDirection::Left);
        } else {
            //throw
        }

        // 端の部分の処理
        if (target.x == width - 1) {
            // ターゲットの真の原座標が右端の場合
            if (current_point(selecting) == waypoint.down()) {
                // selecting が仮の原座標の直下にいる場合
                move_selecting(HVDirection::Left);
                move_selecting(HVDirection::Up);
            }
            move_selecting(HVDirection::Up);
            move_selecting(HVDirection::Right);
            move_selecting(HVDirection::Down);
        } else if (target.y == height - 1) {
            // ターゲットの真の原座標が下端の場合
            if (current_point(selecting) == waypoint.right()) {
                // selecting が仮の原座標の直右にいる場合
                move_selecting(HVDirection::Up);
                move_selecting(HVDirection::Left);
            }
            move_selecting(HVDirection::Left);
            move_selecting(HVDirection::Down);
            move_selecting(HVDirection::Right);
        }

        // ソート済みとする
        sorted_points.push_back(target);
    }
}

void algorithm::brute_force()
{
    // Brute-Force Algorithm
    // とりあえず選択画像を変更しない幅優先探索で

    // TODO: これからやる
}

void algorithm::move_selecting(HVDirection const& direction)
{
    // selecting を指定された方向へ移動する
#ifdef algorithm_debug
    std::cout << "move_selecting direction = " << "URDL"[static_cast<int>(direction)] << std::endl;
#endif

    point_type selecting_cur = current_point(selecting);
    if (direction == HVDirection::Up) {
        assert(selecting_cur.y > sorted_row);
        std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y - 1][selecting_cur.x]);
    } else if (direction == HVDirection::Right) {
        assert(selecting_cur.x < width - 1);
        std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y][selecting_cur.x + 1]);
    } else if (direction == HVDirection::Down) {
        assert(selecting_cur.y < height - 1);
        std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y + 1][selecting_cur.x]);
    } else if (direction == HVDirection::Left) {
        assert(selecting_cur.x > sorted_col);
        std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y][selecting_cur.x - 1]);
    } else {
        //throw
    }

    answer.list.back().change_list.push_back(direction);
#ifdef algorithm_debug
    print();
#endif
}

void algorithm::move_target(point_type const& target, HVDirection const& direction)
{
    // selecting の操作によって原座標が target である断片画像を指定の方向へ移動させる.
#ifdef algorithm_debug
    std::cout << "move_target target = " << target << " direction = " << "URDL"[static_cast<int>(direction)] << std::endl;
#endif

    // selecting の現在の座標
    point_type selecting_cur = current_point(selecting);

    // target の現在の座標
    point_type target_cur = current_point(target);

    // 移動手順リスト
    std::vector<HVDirection> moving_process;

    // selecting が target に隣接していない場合
    if (selecting_cur.manhattan(target_cur) > 1) {
        if (selecting_cur.direction(target_cur) == AllDirection::UpperRight) {
            if (direction == HVDirection::Up) {
                move_to(target_cur.left());
            } else if (direction == HVDirection::Right) {
                move_to(target_cur.down());
            } else if (direction == HVDirection::Down) {
                move_to(target_cur.down());
            } else {
                move_to(target_cur.left());
            }
        } else if (selecting_cur.direction(target_cur) == AllDirection::DownerRight) {
            if (direction == HVDirection::Up) {
                move_to(target_cur.up());
            } else if (direction == HVDirection::Right) {
                move_to(target_cur.up());
            } else if (direction == HVDirection::Down) {
                move_to(target_cur.left());
            } else {
                move_to(target_cur.left());
            }
        } else if (selecting_cur.direction(target_cur) == AllDirection::DownerLeft) {
            if (direction == HVDirection::Up) {
                move_to(target_cur.up());
            } else if (direction == HVDirection::Right) {
                move_to(target_cur.right());
            } else if (direction == HVDirection::Down) {
                move_to(target_cur.right());
            } else {
                move_to(target_cur.up());
            }
        } else if (selecting_cur.direction(target_cur) == AllDirection::UpperLeft) {
            if (direction == HVDirection::Up) {
                move_to(target_cur.right());
            } else if (direction == HVDirection::Right) {
                move_to(target_cur.right());
            } else if (direction == HVDirection::Down) {
                move_to(target_cur.down());
            } else {
                move_to(target_cur.down());
            }
        } else if (selecting_cur.direction(target_cur) == AllDirection::Up) {
            move_to(target_cur.down());
        } else if (selecting_cur.direction(target_cur) == AllDirection::Right) {
            move_to(target_cur.left());
        } else if (selecting_cur.direction(target_cur) == AllDirection::Down) {
            move_to(target_cur.up());
        } else if (selecting_cur.direction(target_cur) == AllDirection::Left) {
            move_to(target_cur.right());
        } else {
            //throw
        }
    }

    selecting_cur = current_point(selecting);
    if (direction == HVDirection::Up) {
        if (selecting_cur.up() == target_cur) {
            if (target_cur.x == width - 1) {
                moving_process = {HVDirection::Left, HVDirection::Up, HVDirection::Up, HVDirection::Right, HVDirection::Down};
            } else {
                moving_process = {HVDirection::Right, HVDirection::Up, HVDirection::Up, HVDirection::Left, HVDirection::Down};
            }
        } else if (selecting_cur.right() == target_cur) {
            if (is_sorted(selecting_cur.up())) {
                moving_process = {HVDirection::Down, HVDirection::Right, HVDirection::Right, HVDirection::Up, HVDirection::Up, HVDirection::Left, HVDirection::Down};
            } else {
                moving_process = {HVDirection::Up, HVDirection::Right, HVDirection::Down};
            }
        } else if (selecting_cur.down() == target_cur) {
            moving_process = {HVDirection::Down};
        } else if (selecting_cur.left() == target_cur) {
            if (is_sorted(selecting_cur.up())) {
                moving_process = {HVDirection::Down, HVDirection::Left, HVDirection::Left, HVDirection::Up, HVDirection::Up, HVDirection::Right, HVDirection::Down};
            } else {
                moving_process = {HVDirection::Up, HVDirection::Left, HVDirection::Down};
            }
        } else {
            //throw
        }
    } else if (direction == HVDirection::Right) {
        if (selecting_cur.up() == target_cur) {
            if (is_sorted(selecting_cur.right())) {
                moving_process = {HVDirection::Left, HVDirection::Up, HVDirection::Up, HVDirection::Right, HVDirection::Right, HVDirection::Down, HVDirection::Left};
            } else {
                moving_process = {HVDirection::Right, HVDirection::Up, HVDirection::Left};
            }
        } else if (selecting_cur.right() == target_cur) {
            if (target_cur.y == height - 1) {
                moving_process = {HVDirection::Up, HVDirection::Right, HVDirection::Right, HVDirection::Down, HVDirection::Left};
            } else {
                moving_process = {HVDirection::Down, HVDirection::Right, HVDirection::Right, HVDirection::Up, HVDirection::Left};
            }
        } else if (selecting_cur.down() == target_cur) {
            if (is_sorted(selecting_cur.right())) {
                moving_process = {HVDirection::Left, HVDirection::Down, HVDirection::Down, HVDirection::Right, HVDirection::Right, HVDirection::Up, HVDirection::Left};
            } else {
                moving_process = {HVDirection::Right, HVDirection::Down, HVDirection::Left};
            }
        } else if (selecting_cur.left() == target_cur) {
            moving_process = {HVDirection::Left};
        } else {
            //throw
        }
    } else if (direction == HVDirection::Down) {
        if (selecting_cur.up() == target_cur) {
            moving_process = {HVDirection::Up};
        } else if (selecting_cur.right() == target_cur) {
            if (is_sorted(selecting_cur.down())) {
                moving_process = {HVDirection::Up, HVDirection::Right, HVDirection::Right, HVDirection::Down, HVDirection::Down, HVDirection::Left, HVDirection::Up};
            } else {
                moving_process = {HVDirection::Down, HVDirection::Right, HVDirection::Up};
            }
        } else if (selecting_cur.down() == target_cur) {
            if (target_cur.x == width - 1) {
                moving_process = {HVDirection::Left, HVDirection::Down, HVDirection::Down, HVDirection::Right, HVDirection::Up};
            } else {
                moving_process = {HVDirection::Right, HVDirection::Down, HVDirection::Down, HVDirection::Left, HVDirection::Up};
            }
        } else if (selecting_cur.left() == target_cur) {
            if (is_sorted(selecting_cur.down())) {
                moving_process = {HVDirection::Up, HVDirection::Left, HVDirection::Left, HVDirection::Down, HVDirection::Down, HVDirection::Right, HVDirection::Up};
            } else {
                moving_process = {HVDirection::Down, HVDirection::Left, HVDirection::Up};
            }
        } else {
            //throw
        }
    } else if (direction == HVDirection::Left) {
        if (selecting_cur.up() == target_cur) {
            if (is_sorted(selecting_cur.left())) {
                moving_process = {HVDirection::Right, HVDirection::Up, HVDirection::Up, HVDirection::Left, HVDirection::Left, HVDirection::Down, HVDirection::Right};
            } else {
                moving_process = {HVDirection::Left, HVDirection::Up, HVDirection::Right};
            }
        } else if (selecting_cur.right() == target_cur) {
            moving_process = {HVDirection::Right};
        } else if (selecting_cur.down() == target_cur) {
            if (is_sorted(selecting_cur.left())) {
                moving_process = {HVDirection::Right, HVDirection::Down, HVDirection::Down, HVDirection::Left, HVDirection::Left, HVDirection::Up, HVDirection::Right};
            } else {
                moving_process = {HVDirection::Left, HVDirection::Down, HVDirection::Right};
            }
        } else if (selecting_cur.left() == target_cur) {
            if (target_cur.y == height - 1) {
                moving_process = {HVDirection::Up, HVDirection::Left, HVDirection::Left, HVDirection::Down, HVDirection::Right};
            } else {
                moving_process = {HVDirection::Down, HVDirection::Left, HVDirection::Left, HVDirection::Up, HVDirection::Right};
            }
        } else {
            //throw
        }
    } else {
        //throw
    }

    sequential_move(selecting_cur, moving_process);
}

void algorithm::sequential_move(point_type const& target, std::vector<HVDirection> const& directions)
{
    // move_selecting を連続して行う

    // target の現在の座標
    point_type target_cur = current_point(target);

    for (auto direction : directions) {
        move_selecting(direction);
        if (direction == HVDirection::Up) {
            target_cur = point_type{target_cur.x, target_cur.y - 1};
        } else if (direction == HVDirection::Right) {
            target_cur = point_type{target_cur.x + 1, target_cur.y};
        } else if (direction == HVDirection::Down) {
            target_cur = point_type{target_cur.x, target_cur.y + 1};
        } else if (direction == HVDirection::Left) {
            target_cur = point_type{target_cur.x - 1, target_cur.y};
        } else {
            //throw
        }
    }
}

void algorithm::move_to(point_type const& to)
{
    // selecting を to まで移動させる
    point_type selecting_cur = current_point(selecting);
    point_type diff = to - selecting_cur;
    AllDirection direction = selecting_cur.direction(to);
    std::cout << "move_to to = " << to << " direction = " << std::vector<std::string>{"S", "U", "UR", "R", "DR", "D", "DL", "L", "UL"}[static_cast<int>(direction)] << std::endl;

    // NOTE: 斜め方向の移動の際は近傍のソート済みの断片画像の存在の有無で縦横の移動の先後を決めているが,
    //       近傍にソート済み断片画像が存在しない場合に何か判断基準はあるだろうか？
    if (direction == AllDirection::Up) {
        for (int i = diff.y; i < 0; ++i) {
            move_selecting(HVDirection::Up);
        }
    } else if (direction == AllDirection::UpperRight) {
        if (is_sorted(selecting_cur.up())) {
            for (int i = diff.x; i > 0; --i) {
                move_selecting(HVDirection::Right);
            }
            for (int i = diff.y; i < 0; ++i) {
                move_selecting(HVDirection::Up);
            }
        } else {
            for (int i = diff.y; i < 0; ++i) {
                move_selecting(HVDirection::Up);
            }
            for (int i = diff.x; i > 0; --i) {
                move_selecting(HVDirection::Right);
            }
        }
    } else if (direction == AllDirection::Right) {
        for (int i = diff.x; i > 0; --i) {
            move_selecting(HVDirection::Right);
        }
    } else if (direction == AllDirection::DownerRight) {
        if (is_sorted(selecting_cur.down())) {
            for (int i = diff.x; i > 0; --i) {
                move_selecting(HVDirection::Right);
            }
            for (int i = diff.y; i > 0; --i) {
                move_selecting(HVDirection::Down);
            }
        } else {
            for (int i = diff.y; i > 0; --i) {
                move_selecting(HVDirection::Down);
            }
            for (int i = diff.x; i > 0; --i) {
                move_selecting(HVDirection::Right);
            }
        }
    } else if (direction == AllDirection::Down) {
        for (int i = diff.y; i > 0; --i) {
            move_selecting(HVDirection::Down);
        }
    } else if (direction == AllDirection::DownerLeft) {
        if (is_sorted(selecting_cur.down())) {
            for (int i = diff.x; i < 0; ++i) {
                move_selecting(HVDirection::Left);
            }
            for (int i = diff.y; i > 0; --i) {
                move_selecting(HVDirection::Down);
            }
        } else {
            for (int i = diff.y; i > 0; --i) {
                move_selecting(HVDirection::Down);
            }
            for (int i = diff.x; i < 0; ++i) {
                move_selecting(HVDirection::Left);
            }
        }
    } else if (direction == AllDirection::Left) {
        for (int i = diff.x; i < 0; ++i) {
            move_selecting(HVDirection::Left);
        }
    } else if (direction == AllDirection::UpperLeft) {
        if (is_sorted(selecting_cur.up())) {
            for (int i = diff.x; i < 0; ++i) {
                move_selecting(HVDirection::Left);
            }
            for (int i = diff.y; i < 0; ++i) {
                move_selecting(HVDirection::Up);
            }
        } else {
            for (int i = diff.y; i < 0; ++i) {
                move_selecting(HVDirection::Up);
            }
            for (int i = diff.x; i < 0; ++i) {
                move_selecting(HVDirection::Left);
            }
        }
    } else {
        //throw
    }
}

inline const bool algorithm::is_sorted(point_type const& point) const
{
    return std::find(sorted_points.begin(), sorted_points.end(), point) != sorted_points.end();
}

#ifdef algorithm_debug
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
    std::cout << "sorted : [ ";
    for (auto point : sorted_points) {
        std::cout << point << " ";
    }
    std::cout << "]" << std::endl;
    std::cout << std::endl;
    std::cout << "answer:" << std::endl;
    std::cout << answer.str();
    std::cin.ignore();
}
#endif

#ifdef algorithm_debug
// 検証用main
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

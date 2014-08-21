//#define algorithm_debug
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cassert>
#include "algorithm.hpp"
//#include "network.hpp"

// 幅優先探索に切り換えるタイミング
// 2 にすると速い
#define BFS_NUM 3

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
    std::unordered_set<point_type> sorted_points;

    // 移動に用いる断片画像の原座標
    // 右下を選んでおけば間違いない
    selecting = point_type{width - 1, height - 1};

    answer.list.push_back(answer_line{current_point(selecting), std::vector<HVDirection>()});

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
        // 残りが BFS_NUM x BFS_NUM の場合は Brute-Force
        if (height - sorted_row <= BFS_NUM && width - sorted_col <= BFS_NUM) {
            brute_force();
            break;
        }

        // 貪欲法を適用
        greedy();

        // ソート済みマーク
        // ここで同時に縮めなければ長方形でも動くはず
        if (height - sorted_row > BFS_NUM) {
            ++sorted_row;
        }
        if (width - sorted_col > BFS_NUM) {
            ++sorted_col;
        }
    }

    // TODO: 送信するなり返すなりする
    std::cout << answer.str() << std::endl;
    return;
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
            i = 0;
            do {
                if (i % 2 == 0 && !is_sorted(current_point(target).up())) {
                    move_target(target, HVDirection::Up);
                    ++i;
                } else if (!is_sorted(current_point(target).right())) {
                    move_target(target, HVDirection::Right);
                }
            } while (current_point(target).direction(waypoint) == AllDirection::UpperRight);
        } else if (current_point(target).direction(waypoint) == AllDirection::DownerRight) {
            i = 0;
            do {
                if (i % 2 == 0 && !is_sorted(current_point(target).down())) {
                    move_target(target, HVDirection::Down);
                    ++i;
                } else if (!is_sorted(current_point(target).right())) {
                    move_target(target, HVDirection::Right);
                }
            } while (current_point(target).direction(waypoint) == AllDirection::DownerRight);
        } else if (current_point(target).direction(waypoint) == AllDirection::DownerLeft) {
            i = 0;
            do {
                if (i % 2 == 0 && !is_sorted(current_point(target).down())) {
                    move_target(target, HVDirection::Down);
                    ++i;
                } else if (!is_sorted(current_point(target).left())) {
                    move_target(target, HVDirection::Left);
                }
            } while (current_point(target).direction(waypoint) == AllDirection::DownerLeft);
        } else if (current_point(target).direction(waypoint) == AllDirection::UpperLeft) {
            i = 0;
            do {
                if (i % 2 == 0 && !is_sorted(current_point(target).up())) {
                    move_target(target, HVDirection::Up);
                    ++i;
                } else if (!is_sorted(current_point(target).left())) {
                    move_target(target, HVDirection::Left);
                }
            } while (current_point(target).direction(waypoint) == AllDirection::UpperLeft);
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
        }

        // 端の部分の処理
        if (target.x == width - 1) {
            // ターゲットの真の原座標が右端の場合
            if (current_point(selecting) == waypoint.up().left()) {
                // selecting が仮の原座標の左上にいる場合
                move_selecting(HVDirection::Right);
                move_selecting(HVDirection::Down);
            } else {
                if (current_point(selecting) == waypoint.down()) {
                    // selecting が仮の原座標の直下にいる場合
                    move_selecting(HVDirection::Left);
                    move_selecting(HVDirection::Up);
                }
                move_selecting(HVDirection::Up);
                move_selecting(HVDirection::Right);
                move_selecting(HVDirection::Down);
            }
        } else if (target.y == height - 1) {
            // ターゲットの真の原座標が下端の場合
            if (current_point(selecting) == waypoint.left().up()) {
                // selecting が仮の原座標の左上にいる場合
                move_selecting(HVDirection::Down);
                move_selecting(HVDirection::Right);
            } else {
                if (current_point(selecting) == waypoint.right()) {
                    // selecting が仮の原座標の直右にいる場合
                    move_selecting(HVDirection::Up);
                    move_selecting(HVDirection::Left);
                }
                move_selecting(HVDirection::Left);
                move_selecting(HVDirection::Down);
                move_selecting(HVDirection::Right);
            }
        }

        // ソート済みとする
        sorted_points.insert(target);
    }
}

void algorithm::brute_force()
{
    // Brute-Force Algorithm
    // 選択画像を変更しない幅優先探索

    // メモリの超無駄遣い！
    std::queue<step_type> open;
    std::unordered_set<step_type> closed;

    // 毎度ポップするやつ
    step_type current;

    // 根ノードをキューに追加
    open.push(step_type{answer, current_point(selecting), matrix});

    // 解答発見フラグ
    bool finished = false;

    while (open.size() > 0) {
        if (is_finished(open.back().matrix)) {
            // 終了
            answer = open.back().answer;
            finished = true;
            break;
        } else {
            current = open.front();
            open.pop();
            if (closed.find(current) != closed.end()) {
                continue;
            }

            if (current.selecting_cur.y == height - BFS_NUM) {
                if (current.selecting_cur.x == width - BFS_NUM) {
                    // 右と下
                    open.push(move_bf(current, HVDirection::Right));
                    open.push(move_bf(current, HVDirection::Down));
                } else if (current.selecting_cur.x == width - 1) {
                    // 左と下
                    open.push(move_bf(current, HVDirection::Left));
                    open.push(move_bf(current, HVDirection::Down));
                } else {
                    // 左右下
                    open.push(move_bf(current, HVDirection::Left));
                    open.push(move_bf(current, HVDirection::Right));
                    open.push(move_bf(current, HVDirection::Down));
                }
            } else if (current.selecting_cur.y == height - 1) {
                if (current.selecting_cur.x == width - BFS_NUM) {
                    // 右と上
                    open.push(move_bf(current, HVDirection::Right));
                    open.push(move_bf(current, HVDirection::Up));
                } else if (current.selecting_cur.x == width - 1) {
                    // 左と上
                    open.push(move_bf(current, HVDirection::Left));
                    open.push(move_bf(current, HVDirection::Up));
                } else {
                    // 左右上
                    open.push(move_bf(current, HVDirection::Left));
                    open.push(move_bf(current, HVDirection::Right));
                    open.push(move_bf(current, HVDirection::Up));
                }
            } else {
                if (current.selecting_cur.x == width - BFS_NUM) {
                    // 右上下
                    open.push(move_bf(current, HVDirection::Right));
                    open.push(move_bf(current, HVDirection::Up));
                    open.push(move_bf(current, HVDirection::Down));
                } else if (current.selecting_cur.x == width - 1) {
                    // 左上下
                    open.push(move_bf(current, HVDirection::Left));
                    open.push(move_bf(current, HVDirection::Up));
                    open.push(move_bf(current, HVDirection::Down));
                } else {
                    // 四方
                    open.push(move_bf(current, HVDirection::Up));
                    open.push(move_bf(current, HVDirection::Right));
                    open.push(move_bf(current, HVDirection::Down));
                    open.push(move_bf(current, HVDirection::Left));
                }
            }
            closed.insert(current);
        }
    }

    if (!finished) {
        std::cout << "NOT FOUND" << std::endl;
    }

    return;
}

const step_type algorithm::move_bf(step_type step, HVDirection const& direction) const
{
    if (direction == HVDirection::Up) {
        std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y - 1][step.selecting_cur.x]);
        step.selecting_cur.y -= 1;
    } else if (direction == HVDirection::Right) {
        std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y][step.selecting_cur.x + 1]);
        step.selecting_cur.x += 1;
    } else if (direction == HVDirection::Down) {
        std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y + 1][step.selecting_cur.x]);
        step.selecting_cur.y += 1;
    } else {
        std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y][step.selecting_cur.x - 1]);
        step.selecting_cur.x -= 1;
    }
    step.answer.list.back().change_list.push_back(direction);
    return step;
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
            throw std::runtime_error("多分ここに入ることはありえないと思う");
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
            throw std::runtime_error("多分ここに入ることはありえないと思う");
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
            throw std::runtime_error("多分ここに入ることはありえないと思う");
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
            throw std::runtime_error("多分ここに入ることはありえないと思う");
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
            throw std::runtime_error("多分ここに入ることはありえないと思う");
        }
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
        }
    }
}

void algorithm::move_to(point_type const& to)
{
    // selecting を to まで移動させる
    point_type selecting_cur = current_point(selecting);
    point_type diff = to - selecting_cur;
    AllDirection direction = selecting_cur.direction(to);

    point_type tmp;

    // NOTE: 斜め方向の移動の際は近傍のソート済みの断片画像の存在の有無で縦横の移動の先後を決めているが,
    //       近傍にソート済み断片画像が存在しない場合に何か判断基準はあるだろうか？
    // TODO: 同じコード使い回しだしどうにからないか
    if (direction == AllDirection::Up) {
        for (int i = diff.y; i < 0; ++i) {
            move_selecting(HVDirection::Up);
        }
    } else if (direction == AllDirection::UpperRight) {
        tmp = selecting_cur.up();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
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
        tmp = selecting_cur.down();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
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
        tmp = selecting_cur.down();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
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
        tmp = selecting_cur.up();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
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
        throw std::runtime_error("多分ここに入ることはありえないと思う");
    }
}

inline const bool algorithm::is_sorted(point_type const& point) const
{
    return sorted_points.find(point) != sorted_points.end();
}

const bool algorithm::is_finished(std::vector<std::vector<point_type>> const& mat) const
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (!(mat[y][x] == point_type{x, y})) {
                return false;
            }
        }
    }
    return true;
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
    /*
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
    */
    std::cin.ignore();
}
#endif

#ifdef algorithm_debug
// 検証用main
// g++ -std=c++11 -I../include algorithm.cpp
int main(int argc, char* argv[])
{
    const auto matrix = std::vector<std::vector<point_type>>{
        std::vector<point_type>{point_type{6,3},point_type{3,1},point_type{11,4},point_type{13,8},point_type{10,7},point_type{10,2},point_type{2,12},point_type{6,6},point_type{3,14},point_type{4,7},point_type{1,9},point_type{11,6},point_type{15,10},point_type{15,9},point_type{12,9},point_type{7,2}},std::vector<point_type>{point_type{11,8},point_type{8,12},point_type{7,9},point_type{13,5},point_type{3,13},point_type{5,12},point_type{10,8},point_type{3,2},point_type{5,4},point_type{15,14},point_type{12,4},point_type{9,11},point_type{6,8},point_type{14,6},point_type{1,2},point_type{11,2}},std::vector<point_type>{point_type{10,12},point_type{14,15},point_type{9,10},point_type{4,10},point_type{2,4},point_type{8,0},point_type{13,2},point_type{10,6},point_type{3,9},point_type{15,7},point_type{9,13},point_type{11,14},point_type{6,13},point_type{12,11},point_type{9,1},point_type{13,4}},std::vector<point_type>{point_type{3,15},point_type{13,12},point_type{13,6},point_type{11,10},point_type{4,14},point_type{2,9},point_type{14,5},point_type{5,8},point_type{7,14},point_type{3,12},point_type{9,14},point_type{12,0},point_type{3,11},point_type{13,10},point_type{12,15},point_type{0,12}},std::vector<point_type>{point_type{14,13},point_type{10,0},point_type{2,2},point_type{5,10},point_type{8,6},point_type{3,3},point_type{15,8},point_type{10,15},point_type{12,5},point_type{6,2},point_type{12,10},point_type{14,0},point_type{11,1},point_type{4,5},point_type{2,11},point_type{9,3}},std::vector<point_type>{point_type{6,10},point_type{0,9},point_type{13,9},point_type{2,13},point_type{6,11},point_type{5,7},point_type{0,5},point_type{7,13},point_type{15,12},point_type{9,6},point_type{10,4},point_type{8,3},point_type{3,0},point_type{2,14},point_type{12,7},point_type{1,13}},std::vector<point_type>{point_type{14,8},point_type{15,11},point_type{5,5},point_type{3,4},point_type{14,9},point_type{6,12},point_type{0,0},point_type{12,3},point_type{0,6},point_type{10,5},point_type{12,8},point_type{11,5},point_type{15,1},point_type{1,15},point_type{1,3},point_type{4,11}},std::vector<point_type>{point_type{15,3},point_type{10,11},point_type{0,7},point_type{11,7},point_type{4,8},point_type{9,8},point_type{3,7},point_type{2,1},point_type{14,3},point_type{0,10},point_type{4,9},point_type{14,12},point_type{2,6},point_type{7,8},point_type{4,2},point_type{0,2}},std::vector<point_type>{point_type{14,7},point_type{9,12},point_type{9,4},point_type{14,11},point_type{1,11},point_type{4,6},point_type{12,12},point_type{6,4},point_type{8,10},point_type{4,4},point_type{10,14},point_type{10,9},point_type{7,0},point_type{5,6},point_type{8,8},point_type{14,2}},std::vector<point_type>{point_type{2,7},point_type{12,6},point_type{8,5},point_type{2,3},point_type{7,4},point_type{11,12},point_type{15,4},point_type{9,2},point_type{2,8},point_type{15,6},point_type{13,15},point_type{6,15},point_type{11,3},point_type{15,15},point_type{14,4},point_type{2,0}},std::vector<point_type>{point_type{12,13},point_type{6,0},point_type{7,3},point_type{9,9},point_type{0,14},point_type{15,5},point_type{9,7},point_type{10,1},point_type{0,1},point_type{11,11},point_type{13,14},point_type{12,1},point_type{1,5},point_type{3,8},point_type{1,14},point_type{6,14}},std::vector<point_type>{point_type{14,14},point_type{1,1},point_type{13,7},point_type{5,14},point_type{11,9},point_type{15,13},point_type{6,5},point_type{15,0},point_type{8,13},point_type{1,6},point_type{7,12},point_type{8,14},point_type{5,15},point_type{5,0},point_type{8,4},point_type{8,15}},std::vector<point_type>{point_type{3,5},point_type{0,15},point_type{12,14},point_type{8,2},point_type{5,3},point_type{7,11},point_type{7,1},point_type{1,10},point_type{11,15},point_type{4,12},point_type{10,13},point_type{8,11},point_type{2,15},point_type{4,1},point_type{5,2},point_type{4,13}},std::vector<point_type>{point_type{3,6},point_type{5,9},point_type{2,10},point_type{11,0},point_type{6,9},point_type{13,1},point_type{8,1},point_type{5,1},point_type{4,0},point_type{1,4},point_type{1,7},point_type{1,0},point_type{8,9},point_type{0,11},point_type{1,8},point_type{7,6}},std::vector<point_type>{point_type{6,7},point_type{15,2},point_type{11,13},point_type{2,5},point_type{9,15},point_type{8,7},point_type{5,11},point_type{4,3},point_type{14,10},point_type{10,10},point_type{7,5},point_type{4,15},point_type{9,5},point_type{13,11},point_type{0,13},point_type{6,1}},std::vector<point_type>{point_type{0,3},point_type{0,4},point_type{7,10},point_type{9,0},point_type{1,12},point_type{3,10},point_type{0,8},point_type{13,0},point_type{13,3},point_type{12,2},point_type{14,1},point_type{7,15},point_type{7,7},point_type{13,13},point_type{5,13},point_type{10,3}}
    };
    const auto size = std::pair<int, int>(matrix[0].size(), matrix.size());
    constexpr int selectable = 16;
    constexpr int cost_select = 10;
    constexpr int cost_change = 10;
    const auto qdata = question_data(size, selectable, cost_select, cost_change, matrix);
    algorithm()(qdata);
    return 0;
}
#endif

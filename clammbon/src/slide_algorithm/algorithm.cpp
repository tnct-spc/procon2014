﻿#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <queue>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "slide_algorithm/algorithm.hpp"

// class definition {{{1
class algorithm::impl : boost::noncopyable
{
public:
    typedef algorithm::return_type return_type;

    impl() = default;
    virtual ~impl() = default;

    auto get() -> boost::optional<return_type>;
    void reset(question_data const& data);

private:
    auto process() -> boost::optional<return_type>;

    boost::optional<question_data> data_;

    const answer_type solve();
    void greedy();
    void brute_force();
    const point_type current_point(point_type const& point) const;
    bool is_sorted(point_type const& point) const;
    bool is_finished(std::vector<std::vector<point_type>> const& mat) const;
    void move_target(point_type const& target, char const& direction);
    void move_to(point_type const& to);
    bool must_chagne_select(step_type const& step) const;
    void add_step(step_type& step);
    point_type get_point_by_point(point_type const& point) const;
    void shorten_answer();

    template <char T>
    void move_selecting();
    template <char First, char Second, char... Rest>
    void move_selecting();

    template <char T>
    void generate_next_step(step_type step);

    void print(std::vector<std::vector<point_type>> const& mat) const;
    void print(answer_type const& answer) const;
    void print(step_type const& step) const;

    static constexpr int BFS_MAX_SIZE = 3;

    std::vector<std::vector<point_type>> matrix;
    std::unordered_set<point_type> sorted_points;
    answer_type answer;
    point_type selecting;
    point_type selecting_cur;
    std::queue<step_type> open;
    std::unordered_set<step_type> closed;
    std::unordered_map<std::vector<std::vector<point_type>>, step_type> visited;
    int width;
    int height;
    int cost_select;
    int cost_change;
    int selectable;
    int score_select;
    int score_change;
    int sorting_row;
    int sorting_col;
    int bfs_height;
    int bfs_width;
};

#if defined(__GNUC__)
constexpr int algorithm::impl::BFS_MAX_SIZE;
#endif

// interfaces for pointer to implementation idiom {{{1
algorithm::algorithm()
    : pimpl_(new impl())
{
}

algorithm::~algorithm()
{
    delete pimpl_;
}

auto algorithm::get() -> boost::optional<return_type>
{
    return pimpl_->get();
}

void algorithm::reset(question_data const& data)
{
    pimpl_->reset(data);
}

auto algorithm::impl::get() -> boost::optional<return_type>
{
    return this->process();
}

void algorithm::impl::reset(question_data const& data)
{
    data_ = data.clone();
}

// implements {{{1
// process {{{2
auto algorithm::impl::process() -> boost::optional<return_type>
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
    matrix = std::move(data_->block);

    // 幅と高さ
    width = data_->size.first;
    height = data_->size.second;

    // コストとレート
    cost_select = data_->cost_select;
    cost_change = data_->cost_change;
    selectable = data_->selectable;

    bfs_width = std::min(width, BFS_MAX_SIZE);
    bfs_height = std::min(height, BFS_MAX_SIZE);

    // ソート済み行及び列
    // この値の行及び列を含む内側部分を操作する
    sorting_row = 0;
    sorting_col = 0;

    // ソート済み断片画像
    std::unordered_set<point_type> sorted_points;

    // GO
#ifdef _DEBUG
    print(matrix);
#endif
    return solve();
}

// move_selecting {{{2
template <>
void algorithm::impl::move_selecting<'U'>()
{
    assert(selecting_cur.y > sorting_row);
    std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y - 1][selecting_cur.x]);
    --selecting_cur.y;
    answer.list.back().actions.push_back('U');
#ifdef _DEBUG
    std::cerr << "U" << std::endl;
    print(matrix);
#endif
}

template <>
void algorithm::impl::move_selecting<'R'>()
{
    assert(selecting_cur.x < width - 1);
    std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y][selecting_cur.x + 1]);
    ++selecting_cur.x;
    answer.list.back().actions.push_back('R');
#ifdef _DEBUG
    std::cerr << "R" << std::endl;
    print(matrix);
#endif
}

template <>
void algorithm::impl::move_selecting<'D'>()
{
    assert(selecting_cur.y < height - 1);
    std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y + 1][selecting_cur.x]);
    ++selecting_cur.y;
    answer.list.back().actions.push_back('D');
#ifdef _DEBUG
    std::cerr << "D" << std::endl;
    print(matrix);
#endif
}

template <>
void algorithm::impl::move_selecting<'L'>()
{
    assert(selecting_cur.x > sorting_col);
    std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y][selecting_cur.x - 1]);
    --selecting_cur.x;
    answer.list.back().actions.push_back('L');
#ifdef _DEBUG
    std::cerr << "L" << std::endl;
    print(matrix);
#endif
}

template <char First, char Second, char... Rest>
void algorithm::impl::move_selecting()
{
    move_selecting<First>();
    move_selecting<Second, Rest...>();
}

// add_step {{{2
void algorithm::impl::add_step(step_type& step)
{
    if (!visited.count(step.matrix)) {
        visited[step.matrix] = step;
    }
    if (!closed.count(step)) {
        open.push(std::move(step));
    }
}

// generate_next_step {{{2
template <>
void algorithm::impl::generate_next_step<'U'>(step_type step)
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y - 1][step.selecting_cur.x]);
    step.selecting_cur.y -= 1;
    std::string& actions = step.answer.list.back().actions;
    if (step.forward) {
        actions += 'U';
    } else {
        actions = 'D' + actions;
    }
    add_step(step);
}

template <>
void algorithm::impl::generate_next_step<'R'>(step_type step)
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y][step.selecting_cur.x + 1]);
    step.selecting_cur.x += 1;
    std::string& actions = step.answer.list.back().actions;
    if (step.forward) {
        actions += 'R';
    } else {
        actions = 'L' + actions;
    }
    add_step(step);
}

template <>
void algorithm::impl::generate_next_step<'D'>(step_type step)
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y + 1][step.selecting_cur.x]);
    step.selecting_cur.y += 1;
    std::string& actions = step.answer.list.back().actions;
    if (step.forward) {
        actions += 'D';
    } else {
        actions = 'U' + actions;
    }
    add_step(step);
}

template <>
void algorithm::impl::generate_next_step<'L'>(step_type step)
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y][step.selecting_cur.x - 1]);
    step.selecting_cur.x -= 1;
    std::string& actions = step.answer.list.back().actions;
    if (step.forward) {
        actions += 'L';
    } else {
        actions = 'R' + actions;
    }
    add_step(step);
}

// current_point {{{2
const point_type algorithm::impl::current_point(point_type const& point) const
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

// solve {{{2
const answer_type algorithm::impl::solve()
{
    // Ian Parberry 氏のアルゴリズムを長方形に拡張したもの
    // とりあえず1回選択のみ

    selecting = point_type{width - 1, height - 1};
    selecting_cur = current_point(selecting);
    answer.list.push_back(answer_atom{selecting_cur, std::string()});

    for (;;) {
        // 貪欲法を適用
        greedy();

        // 残りが bfs_width x bfs_height の場合は Brute-Force
        if (height - sorting_row <= bfs_height + 1 && width - sorting_col <= bfs_width + 1) {
#ifdef _DEBUG
            std::cout << "start brute_force solving" << std::endl;
#endif
            brute_force();
            break;
        }

        // ソート対象の行と列を内側にしてゆく
        if (height - sorting_row > bfs_height + 1) {
            ++sorting_row;
        }
        if (width - sorting_col > bfs_width + 1) {
            ++sorting_col;
        }
    }

    shorten_answer();

#ifdef _DEBUG
    print(answer);
#endif
    return answer;
}

// greedy {{{2
void algorithm::impl::greedy()
{
    // 貪欲法で解く 一番要の部分

    // ターゲットをキューに入れる
    // 中身は原座標
    std::vector<point_type> target_queue;
    if (height - sorting_row > bfs_height) {
        for (int i = sorting_col; i < width; ++i) {
            target_queue.push_back(point_type{i, sorting_row});
        }
    }
    if (width - sorting_col > bfs_width) {
        for (int i = sorting_row; i < height; ++i) {
            target_queue.push_back(point_type{sorting_col, i});
        }
    }

    // カウンタ
    int i;

    // 仮のターゲット座標, これは原座標ではなく実際の座標
    point_type waypoint;

    for (point_type const& target : target_queue) {
#ifdef _DEBUG
        std::cerr << "loop " << target << std::endl;
#endif
        // 端の部分の処理
        if (is_sorted(target)) {
            continue;
        } else if (target.x == width - 2 || target.y == height - 1) {
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
                    move_target(target, 'U');
                } else if (!is_sorted(current_point(target).right())) {
                    move_target(target, 'R');
                }
                ++i;
            } while (current_point(target).direction(waypoint) == AllDirection::UpperRight);
        } else if (current_point(target).direction(waypoint) == AllDirection::DownerRight) {
            i = 0;
            do {
                if (i % 2 == 0 && !is_sorted(current_point(target).down())) {
                    move_target(target, 'D');
                } else if (!is_sorted(current_point(target).right())) {
                    move_target(target, 'R');
                }
                ++i;
            } while (current_point(target).direction(waypoint) == AllDirection::DownerRight);
        } else if (current_point(target).direction(waypoint) == AllDirection::DownerLeft) {
            i = 0;
            do {
                if (i % 2 == 0 && !is_sorted(current_point(target).down())) {
                    move_target(target, 'D');
                } else if (!is_sorted(current_point(target).left())) {
                    move_target(target, 'L');
                }
                ++i;
            } while (current_point(target).direction(waypoint) == AllDirection::DownerLeft);
        } else if (current_point(target).direction(waypoint) == AllDirection::UpperLeft) {
            i = 0;
            do {
                if (i % 2 == 0 && !is_sorted(current_point(target).up())) {
                    move_target(target, 'U');
                } else if (!is_sorted(current_point(target).left())) {
                    move_target(target, 'L');
                }
                ++i;
            } while (current_point(target).direction(waypoint) == AllDirection::UpperLeft);
        }

        // ここまでで x または y 座標が揃っていることになるので, 真横か真上への移動を行う

        if (current_point(target).direction(waypoint) == AllDirection::Up) {
            do {
                move_target(target, 'U');
            } while (current_point(target).direction(waypoint) == AllDirection::Up);
        } else if (current_point(target).direction(waypoint) == AllDirection::Right) {
            do {
                move_target(target, 'R');
            } while (current_point(target).direction(waypoint) == AllDirection::Right);
        } else if (current_point(target).direction(waypoint) == AllDirection::Down) {
            do {
                move_target(target, 'D');
            } while (current_point(target).direction(waypoint) == AllDirection::Down);
        } else if (current_point(target).direction(waypoint) == AllDirection::Left) {
            do {
                move_target(target, 'L');
            } while (current_point(target).direction(waypoint) == AllDirection::Left);
        }

        sorted_points.insert(target);

        // 端の部分の処理
        if (target.x == width - 1) {
            // ターゲットの真の原座標が右端の場合
#ifdef _DEBUG
            std::cerr << "右端" << std::endl;
#endif
            move_to(waypoint.down().left());
            move_to(waypoint.up().left());
            move_selecting<'R', 'D'>();
#ifdef _DEBUG
            std::cerr << "右端done" << std::endl;
#endif
        } else if (target.y == height - 1) {
            // ターゲットの真の原座標が下端の場合
#ifdef _DEBUG
            std::cerr << "下端" << std::endl;
#endif
            move_to(waypoint.right().up());
            move_to(waypoint.left().up());
            move_selecting<'D', 'R'>();
#ifdef _DEBUG
            std::cerr << "下端done" << std::endl;
#endif
        } else if (waypoint.x == width - 1) {
            if (get_point_by_point(waypoint.left()) == target.right()) {
#ifdef _DEBUG
                std::cerr << "SPECIAL CASE 1 START" << std::endl;
#endif
                move_to(waypoint.left().down());
                move_selecting<'U', 'R', 'D', 'L', 'D', 'R', 'U', 'U', 'L', 'D', 'R', 'D'>();
#ifdef _DEBUG
                std::cerr << "SPECIAL CASE 1 END" << std::endl;
#endif
            } else if (selecting_cur.x == width - 2 && selecting_cur.y == sorting_row && get_point_by_point(waypoint.left().down()) == target.right()) {
#ifdef _DEBUG
                std::cerr << "SPECIAL CASE 2 START" << std::endl;
#endif
                move_selecting<'R', 'D', 'L', 'D', 'R', 'U', 'U', 'L', 'D', 'R', 'D'>();
#ifdef _DEBUG
                std::cerr << "SPECIAL CASE 2 END" << std::endl;
#endif
            }
        } else if (waypoint.y == height - 1) {
            if (get_point_by_point(waypoint.up()) == target.down()) {
#ifdef _DEBUG
                std::cerr << "SPECIAL CASE 3 START" << std::endl;
#endif
                move_to(waypoint.up().right());
                move_selecting<'L', 'D', 'R', 'U', 'R', 'D', 'L', 'L', 'U', 'R', 'D', 'R'>();
#ifdef _DEBUG
                std::cerr << "SPECIAL CASE 3 END" << std::endl;
#endif
            } else if (selecting_cur.y == height - 2 && selecting_cur.x == sorting_col && get_point_by_point(waypoint.up().right()) == target.down()) {
#ifdef _DEBUG
                std::cerr << "SPECIAL CASE 4 START" << std::endl;
#endif
                move_selecting<'D', 'R', 'U', 'R', 'D', 'L', 'L', 'U', 'R', 'D', 'R'>();
#ifdef _DEBUG
                std::cerr << "SPECIAL CASE 4 END" << std::endl;
#endif
            }
        }
    }
}

// brute_force {{{2
void algorithm::impl::brute_force()
{
    // Brute-Force Algorithm
    // 双方向探索

    step_type first_step = {true, answer, matrix[selecting_cur.y][selecting_cur.x], selecting_cur, matrix};
    open.push(std::move(first_step));

    auto goal_matrix = matrix;
    for (int y = height - bfs_height; y < height; ++y) for (int x = width - bfs_width; x < width; ++x) {
        goal_matrix[y][x] = {x, y};
    }

    for (int y = height - bfs_height; y < height; ++y) for (int x = width - bfs_width; x < width; ++x) {
        step_type goal_step = {false, {{{{x, y}, ""}}}, goal_matrix[y][x], {x, y}, goal_matrix};
        open.push(std::move(goal_step));
    }

    // 解答発見フラグ
    bool finished = false;

    while (open.size() > 0) {
        step_type current = std::move(open.front());
        open.pop();

        if (visited.count(current.matrix) && visited.at(current.matrix).forward != current.forward) {
            step_type* backward_p;
            step_type* forward_p;
            if (visited.at(current.matrix).forward) {
                forward_p = &visited.at(current.matrix);
                backward_p = &current;
            } else {
                forward_p = &current;
                backward_p = &visited.at(current.matrix);
            }
            step_type& forward = *forward_p;
            step_type& backward = *backward_p;

            if (backward.selecting == forward.selecting) {
                forward.answer.list.back().actions += backward.answer.list.back().actions;
            } else {
                backward.answer.list.back().position = backward.selecting_cur;
                forward.answer.list.push_back(std::move(backward.answer.list.back()));
            }

            answer = std::move(forward.answer);
            finished = true;
            break;
        }

        if (current.selecting_cur.x == width - bfs_width) {
            generate_next_step<'R'>(current);
        } else if (current.selecting_cur.x == width - 1) {
            generate_next_step<'L'>(current);
        } else {
            generate_next_step<'L'>(current);
            generate_next_step<'R'>(current);
        }

        if (current.selecting_cur.y == height - bfs_height) {
            generate_next_step<'D'>(current);
        } else if (current.selecting_cur.y == height - 1) {
            generate_next_step<'U'>(current);
        } else {
            generate_next_step<'D'>(current);
            generate_next_step<'U'>(current);
        }
        closed.insert(std::move(current));
    }

    if (!finished) {
        throw std::runtime_error("Couldn't solve the problem.");
    }

    return;
}

// move_target {{{2
void algorithm::impl::move_target(point_type const& target, char const& direction)
{
    // selecting の操作によって原座標が target である断片画像を指定の方向へ移動させる.

    // target の現在の座標
    point_type target_cur = current_point(target);

    // 移動手順リスト
    std::vector<char> moving_process;

    // selecting が target に隣接していない場合
    if (selecting_cur.manhattan(target_cur) > 1) {
        if (selecting_cur.direction(target_cur) == AllDirection::UpperRight) {
            if (direction == 'U') {
                move_to(target_cur.left());
            } else if (direction == 'R') {
                move_to(target_cur.down());
            } else if (direction == 'D') {
                move_to(target_cur.down());
            } else {
                move_to(target_cur.left());
            }
        } else if (selecting_cur.direction(target_cur) == AllDirection::DownerRight) {
            if (direction == 'U') {
                move_to(target_cur.up());
            } else if (direction == 'R') {
                move_to(target_cur.up());
            } else if (direction == 'D') {
                move_to(target_cur.left());
            } else {
                move_to(target_cur.left());
            }
        } else if (selecting_cur.direction(target_cur) == AllDirection::DownerLeft) {
            if (direction == 'U') {
                move_to(target_cur.up());
            } else if (direction == 'R') {
                move_to(target_cur.right());
            } else if (direction == 'D') {
                move_to(target_cur.right());
            } else {
                move_to(target_cur.up());
            }
        } else if (selecting_cur.direction(target_cur) == AllDirection::UpperLeft) {
            if (direction == 'U') {
                move_to(target_cur.right());
            } else if (direction == 'R') {
                move_to(target_cur.right());
            } else if (direction == 'D') {
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
            throw std::runtime_error("多分ここに入ることはありえないと思う1");
        }
    }

    if (direction == 'U') {
        if (selecting_cur.up() == target_cur) {
            if (target_cur.x == width - 1) {
                move_selecting<'L', 'U', 'U', 'R', 'D'>();
            } else {
                move_selecting<'R', 'U', 'U', 'L', 'D'>();
            }
        } else if (selecting_cur.right() == target_cur) {
            if (is_sorted(selecting_cur.up())) {
                move_selecting<'D', 'R', 'R', 'U', 'U', 'L', 'D'>();
            } else {
                move_selecting<'U', 'R', 'D'>();
            }
        } else if (selecting_cur.down() == target_cur) {
            move_selecting<'D'>();
        } else if (selecting_cur.left() == target_cur) {
            if (is_sorted(selecting_cur.up())) {
                move_selecting<'D', 'L', 'L', 'U', 'U', 'R', 'D'>();
            } else {
                move_selecting<'U', 'L', 'D'>();
            }
        } else {
            throw std::runtime_error("多分ここに入ることはありえないと思う2");
        }
    } else if (direction == 'R') {
        if (selecting_cur.up() == target_cur) {
            if (is_sorted(selecting_cur.right())) {
                move_selecting<'L', 'U', 'U', 'R', 'R', 'D', 'L'>();
            } else {
                move_selecting<'R', 'U', 'L'>();
            }
        } else if (selecting_cur.right() == target_cur) {
            if (target_cur.y == height - 1) {
                move_selecting<'U', 'R', 'R', 'D', 'L'>();
            } else {
                move_selecting<'D', 'R', 'R', 'U', 'L'>();
            }
        } else if (selecting_cur.down() == target_cur) {
            if (is_sorted(selecting_cur.right())) {
                move_selecting<'L', 'D', 'D', 'R', 'R', 'U', 'L'>();
            } else {
                move_selecting<'R', 'D', 'L'>();
            }
        } else if (selecting_cur.left() == target_cur) {
            move_selecting<'L'>();
        } else {
            throw std::runtime_error("多分ここに入ることはありえないと思う3");
        }
    } else if (direction == 'D') {
        if (selecting_cur.up() == target_cur) {
            move_selecting<'U'>();
        } else if (selecting_cur.right() == target_cur) {
            if (is_sorted(selecting_cur.down())) {
                move_selecting<'U', 'R', 'R', 'D', 'D', 'L', 'U'>();
            } else {
                move_selecting<'D', 'R', 'U'>();
            }
        } else if (selecting_cur.down() == target_cur) {
            if (target_cur.x == width - 1) {
                move_selecting<'L', 'D', 'D', 'R', 'U'>();
            } else {
                move_selecting<'R', 'D', 'D', 'L', 'U'>();
            }
        } else if (selecting_cur.left() == target_cur) {
            if (is_sorted(selecting_cur.down())) {
                move_selecting<'U', 'L', 'L', 'D', 'D', 'R', 'U'>();
            } else {
                move_selecting<'D', 'L', 'U'>();
            }
        } else {
            throw std::runtime_error("多分ここに入ることはありえないと思う4");
        }
    } else if (direction == 'L') {
        if (selecting_cur.up() == target_cur) {
            if (is_sorted(selecting_cur.left())) {
                move_selecting<'R', 'U', 'U', 'L', 'L', 'D', 'R'>();
            } else {
                move_selecting<'L', 'U', 'R'>();
            }
        } else if (selecting_cur.right() == target_cur) {
            move_selecting<'R'>();
        } else if (selecting_cur.down() == target_cur) {
            if (is_sorted(selecting_cur.left())) {
                move_selecting<'R', 'D', 'D', 'L', 'L', 'U', 'R'>();
            } else {
                move_selecting<'L', 'D', 'R'>();
            }
        } else if (selecting_cur.left() == target_cur) {
            if (target_cur.y == height - 1) {
                move_selecting<'U', 'L', 'L', 'D', 'R'>();
            } else {
                move_selecting<'D', 'L', 'L', 'U', 'R'>();
            }
        } else {
            throw std::runtime_error("多分ここに入ることはありえないと思う5");
        }
    }
}

// move_to {{{2
void algorithm::impl::move_to(point_type const& to)
{
    // selecting を to まで移動させる
    point_type diff = to - selecting_cur;
    AllDirection direction = selecting_cur.direction(to);

    point_type tmp;

    // NOTE: 斜め方向の移動の際は近傍のソート済みの断片画像の存在の有無で縦横の移動の先後を決めているが,
    //       近傍にソート済み断片画像が存在しない場合に何か判断基準はあるだろうか？
    // TODO: 同じコード使い回しだしどうにからないか
    if (direction == AllDirection::Up) {
        for (int i = diff.y; i < 0; ++i) {
            move_selecting<'U'>();
        }
    } else if (direction == AllDirection::UpperRight) {
        // 右が優先
        tmp = selecting_cur.right();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
            for (int i = diff.y; i < 0; ++i) {
                move_selecting<'U'>();
            }
            for (int i = diff.x; i > 0; --i) {
                move_selecting<'R'>();
            }
        } else {
            for (int i = diff.x; i > 0; --i) {
                move_selecting<'R'>();
            }
            for (int i = diff.y; i < 0; ++i) {
                move_selecting<'U'>();
            }
        }
    } else if (direction == AllDirection::Right) {
        for (int i = diff.x; i > 0; --i) {
            move_selecting<'R'>();
        }
    } else if (direction == AllDirection::DownerRight) {
        tmp = selecting_cur.down();
        // どっち先でもよし
        if (is_sorted(matrix[tmp.y][tmp.x])) {
            for (int i = diff.x; i > 0; --i) {
                move_selecting<'R'>();
            }
            for (int i = diff.y; i > 0; --i) {
                move_selecting<'D'>();
            }
        } else {
            for (int i = diff.y; i > 0; --i) {
                move_selecting<'D'>();
            }
            for (int i = diff.x; i > 0; --i) {
                move_selecting<'R'>();
            }
        }
    } else if (direction == AllDirection::Down) {
        for (int i = diff.y; i > 0; --i) {
            move_selecting<'D'>();
        }
    } else if (direction == AllDirection::DownerLeft) {
        // 下が優先
        tmp = selecting_cur.down();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
            for (int i = diff.x; i < 0; ++i) {
                move_selecting<'L'>();
            }
            for (int i = diff.y; i > 0; --i) {
                move_selecting<'D'>();
            }
        } else {
            for (int i = diff.y; i > 0; --i) {
                move_selecting<'D'>();
            }
            for (int i = diff.x; i < 0; ++i) {
                move_selecting<'L'>();
            }
        }
    } else if (direction == AllDirection::Left) {
        for (int i = diff.x; i < 0; ++i) {
            move_selecting<'L'>();
        }
    } else if (direction == AllDirection::UpperLeft) {
        // どっち先でもよし
        tmp = selecting_cur.up();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
            for (int i = diff.x; i < 0; ++i) {
                move_selecting<'L'>();
            }
            for (int i = diff.y; i < 0; ++i) {
                move_selecting<'U'>();
            }
        } else {
            for (int i = diff.y; i < 0; ++i) {
                move_selecting<'U'>();
            }
            for (int i = diff.x; i < 0; ++i) {
                move_selecting<'L'>();
            }
        }
    }
}

// is_sorted {{{2
bool algorithm::impl::is_sorted(point_type const& point) const
{
    return sorted_points.count(point) > 0;
}

// is_finished {{{2
bool algorithm::impl::is_finished(std::vector<std::vector<point_type>> const& mat) const
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (mat[y][x] != point_type{x, y}) {
                return false;
            }
        }
    }
    return true;
}

point_type algorithm::impl::get_point_by_point(point_type const& point) const
{
    return matrix[point.y][point.x];
}

// shorten_answer {{{2
void algorithm::impl::shorten_answer()
{
    std::string::size_type pos;
    for (auto& atom : answer.list) {
        while (
            atom.actions.find("UD") != std::string::npos ||
            atom.actions.find("DU") != std::string::npos ||
            atom.actions.find("LR") != std::string::npos ||
            atom.actions.find("RL") != std::string::npos
        ) {
            while ((pos = atom.actions.find("UD")) != std::string::npos) {
                atom.actions.erase(pos, 2);
            }
            while ((pos = atom.actions.find("DU")) != std::string::npos) {
                atom.actions.erase(pos, 2);
            }
            while ((pos = atom.actions.find("LR")) != std::string::npos) {
                atom.actions.erase(pos, 2);
            }
            while ((pos = atom.actions.find("RL")) != std::string::npos) {
                atom.actions.erase(pos, 2);
            }
        }
    }
}

// print {{{2
void algorithm::impl::print(std::vector<std::vector<point_type>> const& mat) const
{
    for (std::vector<point_type> const& row : mat) {
        for (point_type const& point : row) {
            std::cerr << boost::format("%1$02X ") % point.num();
        }
        std::cerr << std::endl;
    }
    std::cerr << std::endl;
}

void algorithm::impl::print(answer_type const& answer) const
{
    std::cerr << answer.serialize() << std::endl;
    std::cerr << std::endl;
}

void algorithm::impl::print(step_type const& step) const
{
    print(step.matrix);
    print(step.answer);
}

// vim: set ts=4 sw=4 et fdm=marker:

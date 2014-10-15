#define NDEBUG
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "algorithm.hpp"

// 幅優先探索に切り換えるタイミング
// 2 にすると速い
#define BFS_NUM 3
typedef std::vector<std::vector<point_type>> matrix_type;
struct evaluate_set_type
{
	matrix_type matrix;
	point_type position;//場所
	point_type content;
	int score;
	std::string direct;
};

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
    void process(boost::coroutines::coroutine<return_type>::push_type& yield);
    void operator() (boost::coroutines::coroutine<return_type>::push_type& yield);

    boost::optional<question_data> data_;
    boost::coroutines::coroutine<return_type>::pull_type co_;

    const answer_type solve();
    void greedy();
    void brute_force();
    const point_type current_point(point_type const& point) const;
    bool is_sorted(point_type const& point) const;
    bool is_finished(std::vector<std::vector<point_type>> const& mat) const;
    void move_target(point_type const& target, char const& direction);
    void move_to(point_type const& to);
    bool must_chagne_select(step_type const& step) const;

    template <char T>
    void move_selecting();
    template <char First, char Second, char... Rest>
    void move_selecting();

    template <char T>
    void add_step(step_type step);

    void print(std::vector<std::vector<point_type>> const& mat) const;
    void print(answer_type const& answer) const;
    void print(step_type const& step) const;

	// yoshikawa
	matrix_type destination(matrix_type const& completion);
	int form_evaluate(matrix_type const& completion, matrix_type const& matrix);
	point_type get_start_point(matrix_type const& completion, matrix_type const& matrix);
	int eval_two_piece(matrix_type const& destination, evaluate_set_type const& eval_set, point_type const& new_position);
	evaluate_set_type try_u(matrix_type const& destination, evaluate_set_type const& eval_set);
	evaluate_set_type try_r(matrix_type const& destination, evaluate_set_type const& eval_set);
	evaluate_set_type try_d(matrix_type const& destination, evaluate_set_type const& eval_set);
	evaluate_set_type try_l(matrix_type const& destination, evaluate_set_type const& eval_set);



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
    int score_select;
    int score_change;
    int sorted_row;
    int sorted_col;
};

// interfaces for Boost.Coroutine {{{1
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
    if(co_ && data_)
    {
        auto&& result = co_.get();
        co_();
        return result;
    }
    return boost::none;
}

void algorithm::impl::reset(question_data const& data)
{
    data_ = data.clone();
    co_   = boost::coroutines::coroutine<return_type>::pull_type(
                boost::bind(&impl::process, this, _1)
                );
}

void algorithm::impl::process(boost::coroutines::coroutine<return_type>::push_type& yield)
{
    // 訳ありで転送するだけの関数
    (*this)(yield);
}

// yoshikawa {{{1
matrix_type algorithm::impl::destination(matrix_type const& completion)
{
	matrix_type answer(completion.size(), std::vector<point_type>(completion.at(0).size()));
	for (int i = 0; i < completion.size(); ++i)
	{
		for (int j = 0; j < completion.at(0).size(); ++j)
		{
			answer[completion[i][j].y][completion[i][j].x] = point_type{ j, i };
		}
	}
	return(std::move(answer));
}

int algorithm::impl::form_evaluate(matrix_type const& completion, matrix_type const& matrix)
{
	int s = 0;
	for (int i = 0; i < matrix.size(); ++i)
	{
		for (int j = 0; j < matrix.at(0).size(); ++j)
		{
			s += std::abs(completion[matrix[i][j].x][matrix[i][j].y].y - i);
			s += std::abs(completion[matrix[i][j].x][matrix[i][j].y].x - j);
		}
	}
	return s;
}

point_type algorithm::impl::get_start_point(matrix_type const& completion, matrix_type const& matrix)
{
	int max_val = 0;
	point_type max_point;//場所

	for (int i = 0; i < matrix.size(); ++i)
	{
		for (int j = 0; j < matrix.at(0).size(); ++j)
		{
			int temp = std::abs(completion[matrix[i][j].x][matrix[i][j].y].y - i) + std::abs(completion[matrix[i][j].x][matrix[i][j].y].x - j);
			if (temp > max_val)
			{
				max_val = temp;
				max_point = point_type{ j, i };
			}
		}
	}
	return max_point;
}

int algorithm::impl::eval_two_piece(matrix_type const& destination, evaluate_set_type const& eval_set, point_type const& new_position)
{
	int s = 0;
	point_type const& content_a = eval_set.matrix[eval_set.position.y][eval_set.position.x];
	//aの新しい場所からの距離
	s += std::abs(destination[content_a.y][content_a.x].x - new_position.x);
	s += std::abs(destination[content_a.y][content_a.x].y - new_position.y);
	//aの古い場所からの距離
	s -= std::abs(destination[content_a.y][content_a.x].x - eval_set.position.x);
	s -= std::abs(destination[content_a.y][content_a.x].y - eval_set.position.y);

	point_type const& content_b = eval_set.matrix[new_position.y][new_position.x];
	//bの新しい場所からの距離
	s += std::abs(destination[content_b.y][content_b.x].x - eval_set.position.x);
	s += std::abs(destination[content_b.y][content_b.x].y - eval_set.position.y);
	//bの古い場所からの距離
	s -= std::abs(destination[content_b.y][content_b.x].x - new_position.x);
	s -= std::abs(destination[content_b.y][content_b.x].y - new_position.y);
	assert(s == -2 || s == 0 || s == 2);
	return s;
}

evaluate_set_type algorithm::impl::try_u(matrix_type const& destination, evaluate_set_type const& eval_set)
{
	point_type const& new_position = { eval_set.position.x, eval_set.position.y - 1 };
	evaluate_set_type return_set = eval_set;
	return_set.score += eval_two_piece(destination, eval_set, new_position);
	std::swap(return_set.matrix[return_set.position.y][return_set.position.x], return_set.matrix[new_position.y][new_position.x]);
	return_set.direct += "U";
	return_set.position = new_position;
	return std::move(return_set);
}

evaluate_set_type algorithm::impl::try_r(matrix_type const& destination, evaluate_set_type const& eval_set)
{
	point_type const& new_position = { eval_set.position.x + 1, eval_set.position.y };
	evaluate_set_type return_set = eval_set;
	return_set.score += eval_two_piece(destination, eval_set, new_position);
	std::swap(return_set.matrix[return_set.position.y][return_set.position.x], return_set.matrix[new_position.y][new_position.x]);
	return_set.direct += "R";
	return_set.position = new_position;
	return std::move(return_set);
}

evaluate_set_type algorithm::impl::try_d(matrix_type const& destination, evaluate_set_type const& eval_set)
{
	point_type const& new_position = { eval_set.position.x, eval_set.position.y + 1 };
	evaluate_set_type return_set = eval_set;
	return_set.score += eval_two_piece(destination, return_set, new_position);
	std::swap(return_set.matrix[return_set.position.y][return_set.position.x], return_set.matrix[new_position.y][new_position.x]);
	return_set.direct += "D";
	return_set.position = new_position;
	return std::move(return_set);
}

evaluate_set_type algorithm::impl::try_l(matrix_type const& destination, evaluate_set_type const& eval_set)
{
	point_type const& new_position = { eval_set.position.x - 1, eval_set.position.y };
	evaluate_set_type return_set = eval_set;
	return_set.score += eval_two_piece(destination, eval_set, new_position);
	std::swap(return_set.matrix[return_set.position.y][return_set.position.x], return_set.matrix[new_position.y][new_position.x]);
	return_set.direct += "L";
	return_set.position = new_position;
	return std::move(return_set);
}

// implements {{{1
// operator() {{{2
void algorithm::impl::operator() (boost::coroutines::coroutine<return_type>::push_type& yield)
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
    matrix = data_->block;

    // 幅と高さ
    width = data_->size.first;
    height = data_->size.second;

    // コストとレート
    cost_select = data_->cost_select;
    cost_change = data_->cost_change;

    // ソート済み行及び列
    // この値の行及び列を含む内側部分を操作する
    sorted_row = 0;
    sorted_col = 0;

    // ソート済み断片画像
    std::unordered_set<point_type> sorted_points;

    // 移動に用いる断片画像の原座標
    // 右下を選んでおけば間違いない
    selecting = point_type{width - 1, height - 1};
    selecting_cur = current_point(selecting);

    answer.list.push_back(answer_atom{selecting_cur, std::string()});

    // GO
#ifndef NDEBUG
    print(matrix);
    print(answer);
#endif
    try {
        yield(solve());
    } catch (std::runtime_error const& e) {
        std::cerr << "Couldn't solve the problem." << std::endl;
    }
}

// move_selecting {{{2
template <>
void algorithm::impl::move_selecting<'U'>()
{
    assert(selecting_cur.y > sorted_row);
    std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y - 1][selecting_cur.x]);
    --selecting_cur.y;
    answer.list.back().actions.push_back('U');
#ifndef NDEBUG
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
#ifndef NDEBUG
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
#ifndef NDEBUG
    print(matrix);
#endif
}

template <>
void algorithm::impl::move_selecting<'L'>()
{
    assert(selecting_cur.x > sorted_col);
    std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y][selecting_cur.x - 1]);
    --selecting_cur.x;
    answer.list.back().actions.push_back('L');
#ifndef NDEBUG
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
template <>
void algorithm::impl::add_step<'U'>(step_type step)
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y - 1][step.selecting_cur.x]);
    step.selecting_cur.y -= 1;
    step.answer.list.back().actions.push_back('U');
    visited[step.matrix] = step;
    if (!closed.count(step)) {
        open.push(std::move(step));
    }
}

template <>
void algorithm::impl::add_step<'R'>(step_type step)
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y][step.selecting_cur.x + 1]);
    step.selecting_cur.x += 1;
    step.answer.list.back().actions.push_back('R');
    visited[step.matrix] = step;
    if (!closed.count(step)) {
        open.push(std::move(step));
    }
}

template <>
void algorithm::impl::add_step<'D'>(step_type step)
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y + 1][step.selecting_cur.x]);
    step.selecting_cur.y += 1;
    step.answer.list.back().actions.push_back('D');
    visited[step.matrix] = step;
    if (!closed.count(step)) {
        open.push(std::move(step));
    }
}

template <>
void algorithm::impl::add_step<'L'>(step_type step)
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y][step.selecting_cur.x - 1]);
    step.selecting_cur.x -= 1;
    step.answer.list.back().actions.push_back('L');
    visited[step.matrix] = step;
    if (!closed.count(step)) {
        open.push(std::move(step));
    }
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

	////

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

#ifndef NDEBUG
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
    for (int i = sorted_col; i < width; ++i) {
        target_queue.push_back(point_type{i, sorted_row});
    }
    for (int i = sorted_row + 1; i < height; ++i) {
        target_queue.push_back(point_type{sorted_col, i});
    }

    // カウンタ
    int i;

    // 仮のターゲット座標, これは原座標ではなく実際の座標
    point_type waypoint;

    for (point_type const& target : target_queue) {
        // 端の部分の処理
        if (sorted_points.count(target)) {
            continue;
        }
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

        // 端の部分の処理
        if (target.x == width - 1) {
            // ターゲットの真の原座標が右端の場合
            if (!(selecting_cur == waypoint.up().left())) {
                if (selecting_cur == waypoint.down()) {
                    // selecting が仮の原座標の直下にいる場合
                    move_selecting<'L', 'U'>();
                }
                move_selecting<'U'>();
            }
            move_selecting<'R', 'D'>();
        } else if (target.y == height - 1) {
            // ターゲットの真の原座標が下端の場合
            if (!(selecting_cur == waypoint.left().up())) {
                if (selecting_cur == waypoint.right()) {
                    // selecting が仮の原座標の直右にいる場合
                    move_selecting<'U', 'L'>();
                }
                move_selecting<'L'>();
            }
            move_selecting<'D', 'R'>();
        } else if (selecting_cur.x > 1 && selecting_cur.y > 1) {
            if (target.x == width - 2 && matrix[selecting_cur.y - 1][selecting_cur.x - 1] == target.right()) {
                move_selecting<'L', 'U', 'R', 'D', 'L', 'D', 'R', 'U', 'U', 'L', 'D', 'R', 'D'>();
            } else if (target.y == height - 2 && matrix[selecting_cur.y - 1][selecting_cur.x - 1] == target.down()) {
                move_selecting<'U', 'L', 'D', 'R', 'U', 'R', 'D', 'L', 'L', 'U', 'R', 'D', 'R'>();
            }
        }

        // ソート済みとする
        sorted_points.insert(target);
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
    for (int y = height - BFS_NUM; y < height; ++y) for (int x = width - BFS_NUM; x < width; ++x) {
        goal_matrix[y][x] = {x, y};
    }

    for (int y = height - BFS_NUM; y < height; ++y) for (int x = width - BFS_NUM; x < width; ++x) {
        step_type goal_step = {false, {{{{x, y}, ""}}}, matrix[y][x], {x, y}, goal_matrix};
        open.push(std::move(goal_step));
    }

    // 解答発見フラグ
    bool finished = false;

    while (open.size() > 0) {
        step_type current = std::move(open.front());
        open.pop();

        if (visited.count(current.matrix)) {
            if (visited.at(current.matrix).direction != current.direction) {
                step_type* backward_p;
                step_type* forward_p;
                if (visited.at(current.matrix).direction == false) {
                    forward_p = &current;
                    backward_p = &visited.at(current.matrix);
                } else {
                    forward_p = &visited.at(current.matrix);
                    backward_p = &current;
                }
                step_type& forward = *forward_p;
                step_type& backward = *backward_p;

                std::reverse(backward.answer.list.front().actions.begin(), backward.answer.list.front().actions.end());
                for (char& action : backward.answer.list.front().actions) {
                    switch (action) {
                        case 'U':
                            action = 'D';
                            break;
                        case 'R':
                            action = 'L';
                            break;
                        case 'D':
                            action = 'U';
                            break;
                        case 'L':
                            action = 'R';
                            break;
                    }
                }
                backward.answer.list.front().position = backward.selecting_cur;
                forward.answer.list.push_back(std::move(backward.answer.list.front()));

                answer = std::move(forward.answer);
                finished = true;
                break;
            }
        }

        if (current.selecting_cur.x == width - BFS_NUM) {
            add_step<'R'>(current);
        } else if (current.selecting_cur.x == width - 1) {
            add_step<'L'>(current);
        } else {
            add_step<'L'>(current);
            add_step<'R'>(current);
        }

        if (current.selecting_cur.y == height - BFS_NUM) {
            add_step<'D'>(current);
        } else if (current.selecting_cur.y == height - 1) {
            add_step<'U'>(current);
        } else {
            add_step<'D'>(current);
            add_step<'U'>(current);
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
            throw std::runtime_error("多分ここに入ることはありえないと思う");
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
            throw std::runtime_error("多分ここに入ることはありえないと思う");
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
            throw std::runtime_error("多分ここに入ることはありえないと思う");
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
            throw std::runtime_error("多分ここに入ることはありえないと思う");
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
            throw std::runtime_error("多分ここに入ることはありえないと思う");
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
        tmp = selecting_cur.up();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
            for (int i = diff.x; i > 0; --i) {
                move_selecting<'R'>();
            }
            for (int i = diff.y; i < 0; ++i) {
                move_selecting<'U'>();
            }
        } else {
            for (int i = diff.y; i < 0; ++i) {
                move_selecting<'U'>();
            }
            for (int i = diff.x; i > 0; --i) {
                move_selecting<'R'>();
            }
        }
    } else if (direction == AllDirection::Right) {
        for (int i = diff.x; i > 0; --i) {
            move_selecting<'R'>();
        }
    } else if (direction == AllDirection::DownerRight) {
        tmp = selecting_cur.down();
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
    } else {
        throw std::runtime_error("多分ここに入ることはありえないと思う");
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
            if (!(mat[y][x] == point_type{x, y})) {
                return false;
            }
        }
    }
    return true;
}

// must_chagne_select {{{2
bool algorithm::impl::must_chagne_select(step_type const& step) const
{
    // 偶置換, 奇置換の判定を行い, 奇置換であれば true を返す
    // http://www.aji.sakura.ne.jp/algorithm/slide_goal.html

    int change = 0;
    std::vector<point_type> linear_matrix;
    std::vector<point_type> original_linear_matrix;

    // step.matrix の右下 BFS_NUM x BFS_NUM 部分を 1 列にする
    std::vector<point_type> row;
    row = step.matrix[height - BFS_NUM];
    std::copy(row.end() - 3, row.end(), std::back_inserter(linear_matrix));
    row = step.matrix[height - BFS_NUM + 1];
    std::copy(row.end() - 3, row.end(), std::back_inserter(linear_matrix));
    std::reverse(linear_matrix.end() - 3, linear_matrix.end());
    row = step.matrix[height - BFS_NUM + 2];
    std::copy(row.end() - 3, row.end(), std::back_inserter(linear_matrix));
    linear_matrix.erase(std::remove(linear_matrix.begin(), linear_matrix.end(), step.matrix[step.selecting_cur.y][step.selecting_cur.x]), linear_matrix.end());

    for (int x = width - BFS_NUM; x < width; ++x) {
        original_linear_matrix.push_back(point_type{x, height - BFS_NUM});
    }
    for (int x = width - 1; x >= width - BFS_NUM; --x) {
        original_linear_matrix.push_back(point_type{x, height - BFS_NUM + 1});
    }
    for (int x = width - BFS_NUM; x < width; ++x) {
        original_linear_matrix.push_back(point_type{x, height - BFS_NUM + 2});
    }
    original_linear_matrix.erase(std::remove(original_linear_matrix.begin(), original_linear_matrix.end(), step.matrix[step.selecting_cur.y][step.selecting_cur.x]), original_linear_matrix.end());

    // original_linear_matrix の配置になるように linear_matrix を並び替える
    for (int i = 0; i < linear_matrix.size(); ++i) {
        if (linear_matrix[i] != original_linear_matrix[i]) {
            change += std::distance(linear_matrix.begin(), std::find(linear_matrix.begin(), linear_matrix.end(), original_linear_matrix[i])) - i;
            linear_matrix.erase(std::remove(linear_matrix.begin(), linear_matrix.end(), original_linear_matrix[i]), linear_matrix.end());
            linear_matrix.insert(linear_matrix.begin() + i, original_linear_matrix[i]);
        }
    }

    return change % 2 == 1;
}

// print {{{2
void algorithm::impl::print(std::vector<std::vector<point_type>> const& mat) const
{
    for (std::vector<point_type> const& row : mat) {
        for (point_type const& point : row) {
            std::cout << boost::format("%1$02X ") % point.num();
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void algorithm::impl::print(answer_type const& answer) const
{
    std::cout << answer.serialize() << std::endl;
    std::cout << std::endl;
}

void algorithm::impl::print(step_type const& step) const
{
    print(step.matrix);
    print(step.answer);
}

// vim: set ts=4 sw=4 et fdm=marker:

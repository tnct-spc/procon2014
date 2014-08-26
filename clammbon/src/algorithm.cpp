//#define algorithm_debug
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "algorithm.hpp"

// 幅優先探索に切り換えるタイミング
// 2 にすると速い
#define BFS_NUM 3

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
    inline void print() const;
    const point_type current_point(point_type const& point) const;
    inline bool is_sorted(point_type const& point) const;
    bool is_finished(std::vector<std::vector<point_type>> const& mat) const;
    void move_target(point_type const& target, HVDirection const& direction);
    void move_to(point_type const& to);

    template <HVDirection T>
    void move_selecting();

    template <HVDirection First>
    void sequential_move(point_type const& target);
    template <HVDirection First, HVDirection Second, HVDirection... Rest>
    void sequential_move(point_type const& target);

    template <HVDirection T>
    const step_type move_bf(step_type step) const;

    std::vector<std::vector<point_type>> matrix;
    std::unordered_set<point_type> sorted_points;
    answer_type answer;
    point_type selecting;
    int width;
    int height;
    int cost_select;
    int cost_change;
    int score_select;
    int score_change;
    int sorted_row;
    int sorted_col;
};

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

    // TODO: "yield(return_type型の解答)"とすれば，呼び出し元に制御が戻り，送信処理を行える．
    //       呼び出し元が再度algorithm::getを呼びだせば，中断されたところから実行される(ハズ)．

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

    answer.list.push_back(answer_line{current_point(selecting), std::vector<HVDirection>()});

    // GO
#ifdef algorithm_debug
    print();
#endif
    yield(solve());
}

void algorithm::impl::process(boost::coroutines::coroutine<return_type>::push_type& yield)
{
    // 訳ありで転送するだけの関数
    (*this)(yield);
}

template <>
void algorithm::impl::move_selecting<HVDirection::Up>()
{
    std::cout << "move_selecting<HVDirection::Up>" << std::endl;
    point_type selecting_cur = current_point(selecting);
    assert(selecting_cur.y > sorted_row);
    std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y - 1][selecting_cur.x]);
    answer.list.back().change_list.push_back(HVDirection::Up);
#ifdef algorithm_debug
    print();
#endif
}

template <>
void algorithm::impl::move_selecting<HVDirection::Right>()
{
    std::cout << "move_selecting<HVDirection::Right>" << std::endl;
    point_type selecting_cur = current_point(selecting);
    assert(selecting_cur.x < width - 1);
    std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y][selecting_cur.x + 1]);
    answer.list.back().change_list.push_back(HVDirection::Right);
#ifdef algorithm_debug
    print();
#endif
}

template <>
void algorithm::impl::move_selecting<HVDirection::Down>()
{
    std::cout << "move_selecting<HVDirection::Down>" << std::endl;
    point_type selecting_cur = current_point(selecting);
    assert(selecting_cur.y < height - 1);
    std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y + 1][selecting_cur.x]);
    answer.list.back().change_list.push_back(HVDirection::Down);
#ifdef algorithm_debug
    print();
#endif
}

template <>
void algorithm::impl::move_selecting<HVDirection::Left>()
{
    std::cout << "move_selecting<HVDirection::Left>" << std::endl;
    point_type selecting_cur = current_point(selecting);
    assert(selecting_cur.x > sorted_col);
    std::swap(matrix[selecting_cur.y][selecting_cur.x], matrix[selecting_cur.y][selecting_cur.x - 1]);
    answer.list.back().change_list.push_back(HVDirection::Left);
#ifdef algorithm_debug
    print();
#endif
}

template <>
const step_type algorithm::impl::move_bf<HVDirection::Up>(step_type step) const
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y - 1][step.selecting_cur.x]);
    step.selecting_cur.y -= 1;
    step.answer.list.back().change_list.push_back(HVDirection::Up);
    return step;
}

template <>
const step_type algorithm::impl::move_bf<HVDirection::Right>(step_type step) const
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y][step.selecting_cur.x + 1]);
    step.selecting_cur.x += 1;
    step.answer.list.back().change_list.push_back(HVDirection::Right);
    return step;
}

template <>
const step_type algorithm::impl::move_bf<HVDirection::Down>(step_type step) const
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y + 1][step.selecting_cur.x]);
    step.selecting_cur.y += 1;
    step.answer.list.back().change_list.push_back(HVDirection::Down);
    return step;
}

template <>
const step_type algorithm::impl::move_bf<HVDirection::Left>(step_type step) const
{
    std::swap(step.matrix[step.selecting_cur.y][step.selecting_cur.x], step.matrix[step.selecting_cur.y][step.selecting_cur.x - 1]);
    step.selecting_cur.x -= 1;
    step.answer.list.back().change_list.push_back(HVDirection::Left);
    return step;
}

template <HVDirection First>
void algorithm::impl::sequential_move(point_type const& target)
{
    move_selecting<First>();
}

template <HVDirection First, HVDirection Second, HVDirection... Rest>
void algorithm::impl::sequential_move(point_type const& target)
{
    std::cout << "sequential_move" << std::endl;
    sequential_move<First>(target);
    sequential_move<Second, Rest...>(target);
}

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

const answer_type algorithm::impl::solve()
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

    return answer;
}

void algorithm::impl::greedy()
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
                move_selecting<HVDirection::Right>();
                move_selecting<HVDirection::Down>();
            } else {
                if (current_point(selecting) == waypoint.down()) {
                    // selecting が仮の原座標の直下にいる場合
                    move_selecting<HVDirection::Left>();
                    move_selecting<HVDirection::Up>();
                }
                move_selecting<HVDirection::Up>();
                move_selecting<HVDirection::Right>();
                move_selecting<HVDirection::Down>();
            }
        } else if (target.y == height - 1) {
            // ターゲットの真の原座標が下端の場合
            if (current_point(selecting) == waypoint.left().up()) {
                // selecting が仮の原座標の左上にいる場合
                move_selecting<HVDirection::Down>();
                move_selecting<HVDirection::Right>();
            } else {
                if (current_point(selecting) == waypoint.right()) {
                    // selecting が仮の原座標の直右にいる場合
                    move_selecting<HVDirection::Up>();
                    move_selecting<HVDirection::Left>();
                }
                move_selecting<HVDirection::Left>();
                move_selecting<HVDirection::Down>();
                move_selecting<HVDirection::Right>();
            }
        }

        // ソート済みとする
        sorted_points.insert(target);
    }
}

void algorithm::impl::brute_force()
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
                    open.push(move_bf<HVDirection::Right>(current));
                    open.push(move_bf<HVDirection::Down>(current));
                } else if (current.selecting_cur.x == width - 1) {
                    // 左と下
                    open.push(move_bf<HVDirection::Left>(current));
                    open.push(move_bf<HVDirection::Down>(current));
                } else {
                    // 左右下
                    open.push(move_bf<HVDirection::Left>(current));
                    open.push(move_bf<HVDirection::Right>(current));
                    open.push(move_bf<HVDirection::Down>(current));
                }
            } else if (current.selecting_cur.y == height - 1) {
                if (current.selecting_cur.x == width - BFS_NUM) {
                    // 右と上
                    open.push(move_bf<HVDirection::Right>(current));
                    open.push(move_bf<HVDirection::Up>(current));
                } else if (current.selecting_cur.x == width - 1) {
                    // 左と上
                    open.push(move_bf<HVDirection::Left>(current));
                    open.push(move_bf<HVDirection::Up>(current));
                } else {
                    // 左右上
                    open.push(move_bf<HVDirection::Left>(current));
                    open.push(move_bf<HVDirection::Right>(current));
                    open.push(move_bf<HVDirection::Up>(current));
                }
            } else {
                if (current.selecting_cur.x == width - BFS_NUM) {
                    // 右上下
                    open.push(move_bf<HVDirection::Right>(current));
                    open.push(move_bf<HVDirection::Up>(current));
                    open.push(move_bf<HVDirection::Down>(current));
                } else if (current.selecting_cur.x == width - 1) {
                    // 左上下
                    open.push(move_bf<HVDirection::Left>(current));
                    open.push(move_bf<HVDirection::Up>(current));
                    open.push(move_bf<HVDirection::Down>(current));
                } else {
                    // 四方
                    open.push(move_bf<HVDirection::Up>(current));
                    open.push(move_bf<HVDirection::Right>(current));
                    open.push(move_bf<HVDirection::Down>(current));
                    open.push(move_bf<HVDirection::Left>(current));
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

void algorithm::impl::move_target(point_type const& target, HVDirection const& direction)
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
                sequential_move<HVDirection::Left, HVDirection::Up, HVDirection::Up, HVDirection::Right, HVDirection::Down>(selecting_cur);
            } else {
                sequential_move<HVDirection::Right, HVDirection::Up, HVDirection::Up, HVDirection::Left, HVDirection::Down>(selecting_cur);
            }
        } else if (selecting_cur.right() == target_cur) {
            if (is_sorted(selecting_cur.up())) {
                sequential_move<HVDirection::Down, HVDirection::Right, HVDirection::Right, HVDirection::Up, HVDirection::Up, HVDirection::Left, HVDirection::Down>(selecting_cur);
            } else {
                sequential_move<HVDirection::Up, HVDirection::Right, HVDirection::Down>(selecting_cur);
            }
        } else if (selecting_cur.down() == target_cur) {
            sequential_move<HVDirection::Down>(selecting_cur);
        } else if (selecting_cur.left() == target_cur) {
            if (is_sorted(selecting_cur.up())) {
                sequential_move<HVDirection::Down, HVDirection::Left, HVDirection::Left, HVDirection::Up, HVDirection::Up, HVDirection::Right, HVDirection::Down>(selecting_cur);
            } else {
                sequential_move<HVDirection::Up, HVDirection::Left, HVDirection::Down>(selecting_cur);
            }
        } else {
            throw std::runtime_error("多分ここに入ることはありえないと思う");
        }
    } else if (direction == HVDirection::Right) {
        if (selecting_cur.up() == target_cur) {
            if (is_sorted(selecting_cur.right())) {
                sequential_move<HVDirection::Left, HVDirection::Up, HVDirection::Up, HVDirection::Right, HVDirection::Right, HVDirection::Down, HVDirection::Left>(selecting_cur);
            } else {
                sequential_move<HVDirection::Right, HVDirection::Up, HVDirection::Left>(selecting_cur);
            }
        } else if (selecting_cur.right() == target_cur) {
            if (target_cur.y == height - 1) {
                sequential_move<HVDirection::Up, HVDirection::Right, HVDirection::Right, HVDirection::Down, HVDirection::Left>(selecting_cur);
            } else {
                sequential_move<HVDirection::Down, HVDirection::Right, HVDirection::Right, HVDirection::Up, HVDirection::Left>(selecting_cur);
            }
        } else if (selecting_cur.down() == target_cur) {
            if (is_sorted(selecting_cur.right())) {
                sequential_move<HVDirection::Left, HVDirection::Down, HVDirection::Down, HVDirection::Right, HVDirection::Right, HVDirection::Up, HVDirection::Left>(selecting_cur);
            } else {
                sequential_move<HVDirection::Right, HVDirection::Down, HVDirection::Left>(selecting_cur);
            }
        } else if (selecting_cur.left() == target_cur) {
            sequential_move<HVDirection::Left>(selecting_cur);
        } else {
            throw std::runtime_error("多分ここに入ることはありえないと思う");
        }
    } else if (direction == HVDirection::Down) {
        if (selecting_cur.up() == target_cur) {
            sequential_move<HVDirection::Up>(selecting_cur);
        } else if (selecting_cur.right() == target_cur) {
            if (is_sorted(selecting_cur.down())) {
                sequential_move<HVDirection::Up, HVDirection::Right, HVDirection::Right, HVDirection::Down, HVDirection::Down, HVDirection::Left, HVDirection::Up>(selecting_cur);
            } else {
                sequential_move<HVDirection::Down, HVDirection::Right, HVDirection::Up>(selecting_cur);
            }
        } else if (selecting_cur.down() == target_cur) {
            if (target_cur.x == width - 1) {
                sequential_move<HVDirection::Left, HVDirection::Down, HVDirection::Down, HVDirection::Right, HVDirection::Up>(selecting_cur);
            } else {
                sequential_move<HVDirection::Right, HVDirection::Down, HVDirection::Down, HVDirection::Left, HVDirection::Up>(selecting_cur);
            }
        } else if (selecting_cur.left() == target_cur) {
            if (is_sorted(selecting_cur.down())) {
                sequential_move<HVDirection::Up, HVDirection::Left, HVDirection::Left, HVDirection::Down, HVDirection::Down, HVDirection::Right, HVDirection::Up>(selecting_cur);
            } else {
                sequential_move<HVDirection::Down, HVDirection::Left, HVDirection::Up>(selecting_cur);
            }
        } else {
            throw std::runtime_error("多分ここに入ることはありえないと思う");
        }
    } else if (direction == HVDirection::Left) {
        if (selecting_cur.up() == target_cur) {
            if (is_sorted(selecting_cur.left())) {
                sequential_move<HVDirection::Right, HVDirection::Up, HVDirection::Up, HVDirection::Left, HVDirection::Left, HVDirection::Down, HVDirection::Right>(selecting_cur);
            } else {
                sequential_move<HVDirection::Left, HVDirection::Up, HVDirection::Right>(selecting_cur);
            }
        } else if (selecting_cur.right() == target_cur) {
            sequential_move<HVDirection::Right>(selecting_cur);
        } else if (selecting_cur.down() == target_cur) {
            if (is_sorted(selecting_cur.left())) {
                sequential_move<HVDirection::Right, HVDirection::Down, HVDirection::Down, HVDirection::Left, HVDirection::Left, HVDirection::Up, HVDirection::Right>(selecting_cur);
            } else {
                sequential_move<HVDirection::Left, HVDirection::Down, HVDirection::Right>(selecting_cur);
            }
        } else if (selecting_cur.left() == target_cur) {
            if (target_cur.y == height - 1) {
                sequential_move<HVDirection::Up, HVDirection::Left, HVDirection::Left, HVDirection::Down, HVDirection::Right>(selecting_cur);
            } else {
                sequential_move<HVDirection::Down, HVDirection::Left, HVDirection::Left, HVDirection::Up, HVDirection::Right>(selecting_cur);
            }
        } else {
            throw std::runtime_error("多分ここに入ることはありえないと思う");
        }
    }
}
void algorithm::impl::move_to(point_type const& to)
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
            move_selecting<HVDirection::Up>();
        }
    } else if (direction == AllDirection::UpperRight) {
        tmp = selecting_cur.up();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
            for (int i = diff.x; i > 0; --i) {
                move_selecting<HVDirection::Right>();
            }
            for (int i = diff.y; i < 0; ++i) {
                move_selecting<HVDirection::Up>();
            }
        } else {
            for (int i = diff.y; i < 0; ++i) {
                move_selecting<HVDirection::Up>();
            }
            for (int i = diff.x; i > 0; --i) {
                move_selecting<HVDirection::Right>();
            }
        }
    } else if (direction == AllDirection::Right) {
        for (int i = diff.x; i > 0; --i) {
            move_selecting<HVDirection::Right>();
        }
    } else if (direction == AllDirection::DownerRight) {
        tmp = selecting_cur.down();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
            for (int i = diff.x; i > 0; --i) {
                move_selecting<HVDirection::Right>();
            }
            for (int i = diff.y; i > 0; --i) {
                move_selecting<HVDirection::Down>();
            }
        } else {
            for (int i = diff.y; i > 0; --i) {
                move_selecting<HVDirection::Down>();
            }
            for (int i = diff.x; i > 0; --i) {
                move_selecting<HVDirection::Right>();
            }
        }
    } else if (direction == AllDirection::Down) {
        for (int i = diff.y; i > 0; --i) {
            move_selecting<HVDirection::Down>();
        }
    } else if (direction == AllDirection::DownerLeft) {
        tmp = selecting_cur.down();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
            for (int i = diff.x; i < 0; ++i) {
                move_selecting<HVDirection::Left>();
            }
            for (int i = diff.y; i > 0; --i) {
                move_selecting<HVDirection::Down>();
            }
        } else {
            for (int i = diff.y; i > 0; --i) {
                move_selecting<HVDirection::Down>();
            }
            for (int i = diff.x; i < 0; ++i) {
                move_selecting<HVDirection::Left>();
            }
        }
    } else if (direction == AllDirection::Left) {
        for (int i = diff.x; i < 0; ++i) {
            move_selecting<HVDirection::Left>();
        }
    } else if (direction == AllDirection::UpperLeft) {
        tmp = selecting_cur.up();
        if (is_sorted(matrix[tmp.y][tmp.x])) {
            for (int i = diff.x; i < 0; ++i) {
                move_selecting<HVDirection::Left>();
            }
            for (int i = diff.y; i < 0; ++i) {
                move_selecting<HVDirection::Up>();
            }
        } else {
            for (int i = diff.y; i < 0; ++i) {
                move_selecting<HVDirection::Up>();
            }
            for (int i = diff.x; i < 0; ++i) {
                move_selecting<HVDirection::Left>();
            }
        }
    } else {
        throw std::runtime_error("多分ここに入ることはありえないと思う");
    }
}

inline bool algorithm::impl::is_sorted(point_type const& point) const
{
    return sorted_points.find(point) != sorted_points.end();
}

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

#ifdef algorithm_debug
inline void algorithm::impl::print() const
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
// g++ -std=c++11 -I../include -lboost_system -lboost_coroutine
int main(int argc, char* argv[])
{
    const auto matrix = std::vector<std::vector<point_type>>{
        std::vector<point_type>{point_type{6,3},point_type{3,1},point_type{11,4},point_type{13,8},point_type{10,7},point_type{10,2},point_type{2,12},point_type{6,6},point_type{3,14},point_type{4,7},point_type{1,9},point_type{11,6},point_type{15,10},point_type{15,9},point_type{12,9},point_type{7,2}},std::vector<point_type>{point_type{11,8},point_type{8,12},point_type{7,9},point_type{13,5},point_type{3,13},point_type{5,12},point_type{10,8},point_type{3,2},point_type{5,4},point_type{15,14},point_type{12,4},point_type{9,11},point_type{6,8},point_type{14,6},point_type{1,2},point_type{11,2}},std::vector<point_type>{point_type{10,12},point_type{14,15},point_type{9,10},point_type{4,10},point_type{2,4},point_type{8,0},point_type{13,2},point_type{10,6},point_type{3,9},point_type{15,7},point_type{9,13},point_type{11,14},point_type{6,13},point_type{12,11},point_type{9,1},point_type{13,4}},std::vector<point_type>{point_type{3,15},point_type{13,12},point_type{13,6},point_type{11,10},point_type{4,14},point_type{2,9},point_type{14,5},point_type{5,8},point_type{7,14},point_type{3,12},point_type{9,14},point_type{12,0},point_type{3,11},point_type{13,10},point_type{12,15},point_type{0,12}},std::vector<point_type>{point_type{14,13},point_type{10,0},point_type{2,2},point_type{5,10},point_type{8,6},point_type{3,3},point_type{15,8},point_type{10,15},point_type{12,5},point_type{6,2},point_type{12,10},point_type{14,0},point_type{11,1},point_type{4,5},point_type{2,11},point_type{9,3}},std::vector<point_type>{point_type{6,10},point_type{0,9},point_type{13,9},point_type{2,13},point_type{6,11},point_type{5,7},point_type{0,5},point_type{7,13},point_type{15,12},point_type{9,6},point_type{10,4},point_type{8,3},point_type{3,0},point_type{2,14},point_type{12,7},point_type{1,13}},std::vector<point_type>{point_type{14,8},point_type{15,11},point_type{5,5},point_type{3,4},point_type{14,9},point_type{6,12},point_type{0,0},point_type{12,3},point_type{0,6},point_type{10,5},point_type{12,8},point_type{11,5},point_type{15,1},point_type{1,15},point_type{1,3},point_type{4,11}},std::vector<point_type>{point_type{15,3},point_type{10,11},point_type{0,7},point_type{11,7},point_type{4,8},point_type{9,8},point_type{3,7},point_type{2,1},point_type{14,3},point_type{0,10},point_type{4,9},point_type{14,12},point_type{2,6},point_type{7,8},point_type{4,2},point_type{0,2}},std::vector<point_type>{point_type{14,7},point_type{9,12},point_type{9,4},point_type{14,11},point_type{1,11},point_type{4,6},point_type{12,12},point_type{6,4},point_type{8,10},point_type{4,4},point_type{10,14},point_type{10,9},point_type{7,0},point_type{5,6},point_type{8,8},point_type{14,2}},std::vector<point_type>{point_type{2,7},point_type{12,6},point_type{8,5},point_type{2,3},point_type{7,4},point_type{11,12},point_type{15,4},point_type{9,2},point_type{2,8},point_type{15,6},point_type{13,15},point_type{6,15},point_type{11,3},point_type{15,15},point_type{14,4},point_type{2,0}},std::vector<point_type>{point_type{12,13},point_type{6,0},point_type{7,3},point_type{9,9},point_type{0,14},point_type{15,5},point_type{9,7},point_type{10,1},point_type{0,1},point_type{11,11},point_type{13,14},point_type{12,1},point_type{1,5},point_type{3,8},point_type{1,14},point_type{6,14}},std::vector<point_type>{point_type{14,14},point_type{1,1},point_type{13,7},point_type{5,14},point_type{11,9},point_type{15,13},point_type{6,5},point_type{15,0},point_type{8,13},point_type{1,6},point_type{7,12},point_type{8,14},point_type{5,15},point_type{5,0},point_type{8,4},point_type{8,15}},std::vector<point_type>{point_type{3,5},point_type{0,15},point_type{12,14},point_type{8,2},point_type{5,3},point_type{7,11},point_type{7,1},point_type{1,10},point_type{11,15},point_type{4,12},point_type{10,13},point_type{8,11},point_type{2,15},point_type{4,1},point_type{5,2},point_type{4,13}},std::vector<point_type>{point_type{3,6},point_type{5,9},point_type{2,10},point_type{11,0},point_type{6,9},point_type{13,1},point_type{8,1},point_type{5,1},point_type{4,0},point_type{1,4},point_type{1,7},point_type{1,0},point_type{8,9},point_type{0,11},point_type{1,8},point_type{7,6}},std::vector<point_type>{point_type{6,7},point_type{15,2},point_type{11,13},point_type{2,5},point_type{9,15},point_type{8,7},point_type{5,11},point_type{4,3},point_type{14,10},point_type{10,10},point_type{7,5},point_type{4,15},point_type{9,5},point_type{13,11},point_type{0,13},point_type{6,1}},std::vector<point_type>{point_type{0,3},point_type{0,4},point_type{7,10},point_type{9,0},point_type{1,12},point_type{3,10},point_type{0,8},point_type{13,0},point_type{13,3},point_type{12,2},point_type{14,1},point_type{7,15},point_type{7,7},point_type{13,13},point_type{5,13},point_type{10,3}}
    };
    const auto size = std::pair<int, int>(matrix[0].size(), matrix.size());
    constexpr int problem_id = 1;
    const std::string player_id = "player_id";
    constexpr int selectable = 16;
    constexpr int cost_select = 10;
    constexpr int cost_change = 10;
    const auto qdata = question_data(problem_id, player_id, size, selectable, cost_select, cost_change, matrix);
    algorithm algo;
    algo.reset(qdata);
    const auto answer = algo.get();
    return 0;
}
#endif

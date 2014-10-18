#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <queue>
#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "truncater.hpp"

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
class truncater::impl : boost::noncopyable
{
public:
    typedef truncater::return_type return_type;

    impl() = default;
    virtual ~impl() = default;

    auto get() -> boost::optional<return_type>;
    void reset(question_data const& data);

private:
    void process(boost::coroutines::coroutine<return_type>::push_type& yield);
    void operator() (boost::coroutines::coroutine<return_type>::push_type& yield);

    boost::optional<question_data> data_;
    boost::coroutines::coroutine<return_type>::pull_type co_;

    point_type ymove();
    int form_evaluate(matrix_type const& mat);
    point_type get_start_point(matrix_type const& mat);
    int eval_two_piece(evaluate_set_type const& eval_set, point_type const& new_position);
    evaluate_set_type try_u(evaluate_set_type return_set);
    evaluate_set_type try_r(evaluate_set_type return_set);
    evaluate_set_type try_d(evaluate_set_type return_set);
    evaluate_set_type try_l(evaluate_set_type return_set);

    matrix_type matrix;
    answer_type answer;
    int width;
    int height;
    int selectable;
};

// interfaces for Boost.Coroutine {{{1
truncater::truncater()
    : pimpl_(new impl())
{
}

truncater::~truncater()
{
    delete pimpl_;
}

auto truncater::get() -> boost::optional<return_type>
{
    return pimpl_->get();
}

void truncater::reset(question_data const& data)
{
    pimpl_->reset(data);
}

auto truncater::impl::get() -> boost::optional<return_type>
{
    if(co_ && data_)
    {
        auto&& result = co_.get();
        co_();
        return result;
    }
    return boost::none;
}

void truncater::impl::reset(question_data const& data)
{
    data_ = data.clone();
    co_   = boost::coroutines::coroutine<return_type>::pull_type(
            boost::bind(&impl::process, this, _1)
            );
}

void truncater::impl::process(boost::coroutines::coroutine<return_type>::push_type& yield)
{
    // 訳ありで転送するだけの関数
    (*this)(yield);
}

// yoshikawa {{{1
int truncater::impl::form_evaluate(matrix_type const& mat)
{
    int s = 0;
    for (int i = 0; i < mat.size(); ++i)
    {
        for (int j = 0; j < mat.at(0).size(); ++j)
        {
            s += mat[i][j].manhattan({ j, i });
        }
    }
    return s;
}

point_type truncater::impl::get_start_point(matrix_type const& mat)
{
    int max_val = 0;
    point_type max_point;//実態
    point_type position;

    for (int i = 0; i < mat.size(); ++i)
    {
        for (int j = 0; j < mat.at(0).size(); ++j)
        {
            int temp = mat[i][j].manhattan({ j, i });
            if (temp > max_val)
            {
                max_val = temp;
                max_point = point_type{ j, i };
                position = point_type{ mat[i][j].x, mat[i][j].y };
            }
        }
    }
    return position;
}

int truncater::impl::eval_two_piece(evaluate_set_type const& eval_set, point_type const& new_position)
{
    int s = 0;
    point_type const& content_a = eval_set.matrix[eval_set.position.y][eval_set.position.x];
    //aの新しい場所からの距離
    s += content_a.manhattan(new_position);
    //aの古い場所からの距離
    s -= content_a.manhattan(eval_set.position);
    point_type const& content_b = eval_set.matrix[new_position.y][new_position.x];
    //bの新しい場所からの距離
    s += content_b.manhattan(eval_set.position);
    //bの古い場所からの距離
    s -= content_b.manhattan(new_position);

    assert(s == -2 || s == 0 || s == 2);
    return s;
}


evaluate_set_type truncater::impl::try_u(evaluate_set_type return_set)
{
    point_type const& new_position = { return_set.position.x, return_set.position.y - 1 };
    return_set.score += eval_two_piece(return_set, new_position);
    std::swap(return_set.matrix[return_set.position.y][return_set.position.x], return_set.matrix[new_position.y][new_position.x]);
    return_set.direct += "U";
    return_set.position = new_position;
    return std::move(return_set);
}

evaluate_set_type truncater::impl::try_r(evaluate_set_type return_set)
{
    point_type const& new_position = { return_set.position.x + 1, return_set.position.y };
    return_set.score += eval_two_piece(return_set, new_position);
    std::swap(return_set.matrix[return_set.position.y][return_set.position.x], return_set.matrix[new_position.y][new_position.x]);
    return_set.direct += "R";
    return_set.position = new_position;
    return std::move(return_set);
}

evaluate_set_type truncater::impl::try_d(evaluate_set_type return_set)
{
    point_type const& new_position = { return_set.position.x, return_set.position.y + 1 };
    return_set.score += eval_two_piece(return_set, new_position);
    std::swap(return_set.matrix[return_set.position.y][return_set.position.x], return_set.matrix[new_position.y][new_position.x]);
    return_set.direct += "D";
    return_set.position = new_position;
    return std::move(return_set);
}

evaluate_set_type truncater::impl::try_l(evaluate_set_type return_set)
{
    point_type const& new_position = { return_set.position.x - 1, return_set.position.y };
    return_set.score += eval_two_piece(return_set, new_position);
    std::swap(return_set.matrix[return_set.position.y][return_set.position.x], return_set.matrix[new_position.y][new_position.x]);
    return_set.direct += "L";
    return_set.position = new_position;
    return std::move(return_set);
}

point_type truncater::impl::ymove()
{
    std::cout << "ymove start" << std::endl;

    std::queue<evaluate_set_type> que;
    std::vector<evaluate_set_type> children;
    children.reserve(900);

	point_type start_position;

	if (selectable == 2)
	{
		start_position = point_type{ width - 1, height - 1 };
	}
	else
	{
		start_position = get_start_point(matrix);
	}
    
	evaluate_set_type start = evaluate_set_type{ matrix, start_position, matrix[start_position.y][start_position.x], form_evaluate(start.matrix), " " };
	evaluate_set_type best = start;
    
    int depth = 0;
    
    que.push(start);

    //std::cout << "select piece position = " << start.position << std::endl;
    //std::cout << "select piece content = " << start.content << std::endl;

    while (que.size() > 0 && depth++ < 100)
    {
        //std::cout << "depth = " << depth++ << " score = " << best.score << " que size = " << que.size() << std::endl;
        while (que.size() > 0)
        {
            auto const node = que.front();
            que.pop();

            if (best.score > node.score) best = node;

            if (node.position.y != 0 && node.direct.back() != 'D')
            {
                auto child = try_u(node);
                if (child.score <= node.score)
                {
                    children.push_back(std::move(child));
                }
            }
            if (node.position.x != width - 1 && node.direct.back() != 'L')
            {
                auto child = try_r(node);
                if (child.score <= node.score)
                {
                    children.push_back(std::move(child));
                }
            }
            if (node.position.y != height - 1 && node.direct.back() != 'U')
            {
                auto child = try_d(node);
                if (child.score <= node.score)
                {
                    children.push_back(std::move(child));
                }
            }
            if (node.position.x != 0 && node.direct.back() != 'R')
            {
                auto child = try_l(node);
                if (child.score <= node.score)
                {
                    children.push_back(std::move(child));
                }
            }
        }
        
        std::sort(children.begin(), children.end(), [](evaluate_set_type a, evaluate_set_type b){return a.score < b.score; });

        for (int i = 0; i < children.size() && i < 100; ++i)
        {
            que.push(children.at(i));
        }
        children.clear();
    }

    matrix = best.matrix;
    answer.list.push_back({ start_position, best.direct.substr(1) });

    std::cout << "ymove done." << std::endl;

	return best.content;
}

// operator() {{{1
void truncater::impl::operator() (boost::coroutines::coroutine<return_type>::push_type& yield)
{
    algorithm algo;
    matrix = data_->block;
    width = data_->size.first;
    height = data_->size.second;
    selectable = data_->selectable;

    if ((width > 3 || height > 3) && selectable >= 3) ymove();

    auto qdata = question_data(
        data_->problem_id,
        data_->player_id,
        data_->size,
        data_->selectable,
        data_->cost_change,
        data_->cost_change,
        std::move(matrix)
    );
    algo.reset(std::move(qdata));
    answer_type subsequent_answer = *algo.get();
    std::copy(subsequent_answer.list.begin(), subsequent_answer.list.end(), std::back_inserter(answer.list));
    yield(std::move(answer));
}

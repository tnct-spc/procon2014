#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <queue>
#include <deque>
#include <random>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "slide_algorithm/truncater.hpp"

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
    auto process() -> boost::optional<return_type>;

    boost::optional<question_data> data_;

    void ymove();
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

// interfaces for pointer to implementation idiom {{{1
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
    return this->process();
}

void truncater::impl::reset(question_data const& data)
{
    data_ = data.clone();
}

// yoshikawa {{{1
int truncater::impl::form_evaluate(matrix_type const& mat)
{
    int s = 0;
    for (int i = 0; i < mat.size(); ++i)
    {
        for (int j = 0; j < mat.at(0).size(); ++j)
        {
            s += mat[i][j].manhattan_pow({ j, i });
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
            int temp = mat[i][j].manhattan_pow({ j, i });
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
    s += content_a.manhattan_pow(new_position);
    //aの古い場所からの距離
    s -= content_a.manhattan_pow(eval_set.position);
    point_type const& content_b = eval_set.matrix[new_position.y][new_position.x];
    //bの新しい場所からの距離
    s += content_b.manhattan_pow(eval_set.position);
    //bの古い場所からの距離
    s -= content_b.manhattan_pow(new_position);

    //assert(s == -2 || s == 0 || s == 2);
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

void truncater::impl::ymove()
{
    std::cout << "ymove start" << std::endl;

    int const width = matrix.at(0).size();
    int const height = matrix.size();

    std::queue<evaluate_set_type> que;
    std::vector<evaluate_set_type> children;
    children.reserve(900);

    point_type const start_position = get_start_point(matrix);
    evaluate_set_type start = evaluate_set_type{ matrix, start_position, matrix[start_position.y][start_position.x], form_evaluate(start.matrix), " " };
    evaluate_set_type best = start;
    
    que.push(start);

    //std::cout << "select piece position = " << start.position << std::endl;
    //std::cout << "select piece content = " << start.content << std::endl;
    
    auto P = [](int e, int ne, double T) {
        if( ne < e ) {
            return 1.0;
        } else {
            return std::exp((e - ne)/T);
        }
    };
    double T = 1000.0;
    
    std::random_device sec_rng;
    std::mt19937 mt(sec_rng());
    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
    auto prob = [&]{return prob_dist(mt);};
    
    auto is_transition = [&](int s, int ns) {
        return prob() < P(s, ns, T);
    };

    while (que.size() > 0 && T > 1.0)
    {
        //std::cout << "T = " << T << ", score = " << best.score << " que size = " << que.size() << std::endl;
        while (que.size() > 0)
        {
            auto const node = que.front();
            que.pop();

            if (best.score > node.score) best = node;

            if (node.position.y != 0 && node.direct.back() != 'D')
            {
                auto child = try_u(node);
                //if (child.score <= node.score)
                if (is_transition(node.score, child.score))
                {
                    children.push_back(std::move(child));
                }
            }
            if (node.position.x != width - 1 && node.direct.back() != 'L')
            {
                auto child = try_r(node);
                //if (child.score <= node.score)
                if (is_transition(node.score, child.score))
                {
                    children.push_back(std::move(child));
                }
            }
            if (node.position.y != height - 1 && node.direct.back() != 'U')
            {
                auto child = try_d(node);
                //if (child.score <= node.score)
                if (is_transition(node.score, child.score))
                {
                    children.push_back(std::move(child));
                }
            }
            if (node.position.x != 0 && node.direct.back() != 'R')
            {
                auto child = try_l(node);
                //if (child.score <= node.score)
                if (is_transition(node.score, child.score))
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
        
        T *= 0.95;
    }

    matrix = best.matrix;
    answer.list.push_back({ start_position, best.direct.substr(1) });

    std::cout << "ymove done." << std::endl;
}

// process {{{1
auto truncater::impl::process() -> boost::optional<return_type>
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
        data_->cost_select,
        data_->cost_change,
        std::move(matrix)
    );
    algo.reset(std::move(qdata));
    answer_type subsequent_answer = *algo.get();
    std::copy(subsequent_answer.list.begin(), subsequent_answer.list.end(), std::back_inserter(answer.list));
    return std::move(answer);
}

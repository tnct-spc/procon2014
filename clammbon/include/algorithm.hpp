#ifndef RESOLVER_ALGORITHM_HPP
#define RESOLVER_ALGORITHM_HPP

#include <boost/optional.hpp>
#include <boost/coroutine/coroutine.hpp>
#include "data_type.hpp"

class algorithm
{
public:
    typedef answer_type return_type; // TODO: あとで相談とか

    algorithm() = default;
    virtual ~algorithm() = default;

    auto get() -> boost::optional<return_type>;
    void reset(question_data const& data);

private:
    void process(boost::coroutines::coroutine<return_type>::push_type& yield);
    void operator() (boost::coroutines::coroutine<return_type>::push_type& yield);

    boost::optional<question_data> data_;
    boost::coroutines::coroutine<return_type>::pull_type co_;

    void solve();
    void greedy();
    void brute_force();
    inline void print() const;
    const point_type current_point(point_type const& point) const;
    inline const bool is_sorted(point_type const& point) const;
    const bool is_finished(std::vector<std::vector<point_type>> const& mat) const;
    void sequential_move(point_type const& target, std::vector<HVDirection> const& directions);
    void move_selecting(HVDirection const& direction);
    void move_target(point_type const& target, HVDirection const& direction);
    void move_to(point_type const& to);
    const step_type move_bf(step_type step, HVDirection const& direction) const;

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

#endif

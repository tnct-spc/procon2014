#ifndef RESOLVER_DATA_TYPE_HPP
#define RESOLVER_DATA_TYPE_HPP

#include <cstdint>
#include <cmath>
#include <tuple>
#include <utility>
#include <vector>
#include <boost/noncopyable.hpp>
#include <opencv2/core/core.hpp>

struct point_type
{
    int x;
    int y;
    
    friend inline bool operator== (point_type const& lhs, point_type const& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
	}
	friend inline bool operator< (point_type const& lhs, point_type const& rhs)
	{
		return (lhs.x == rhs.x) ? lhs.y < rhs.y : lhs.x < rhs.x;
	}
    friend inline point_type const operator- (point_type const& lhs, point_type const& rhs)
    {
        return point_type{lhs.x - rhs.x, lhs.y - rhs.y};
    }

    inline int manhattan(point_type const& other) const
    {
        return std::abs(this->x - other.x) + std::abs(this->y - other.y);
    }
    template<class T = double>
    inline T euclid(point_type const& other) const
    {
        return std::sqrt<T>(
            std::pow<T>(this->x - other.x, 2) + 
            std::pow<T>(this->y - other.y, 2)
            );
    }
};

typedef cv::Vec3b                            pixel_type;
typedef std::vector<uint8_t>                 unfold_image_type;
typedef cv::Mat_<cv::Vec3b>                  image_type;

// [i][j]の位置に分割された画像(cv::Mat_<cv::Vec3b>)が入っている．
typedef std::vector<std::vector<image_type>> split_image_type;

struct question_data : private boost::noncopyable
{
    int problem_id;
    std::string player_id;

    std::pair<int,int> size;
    int selectable;
    int cost_select;
    int cost_change;
    std::vector<std::vector<point_type>> block;

    question_data(
        int const problem_id,
        std::string const& player_id,
        std::pair<int,int> const& size,
        int const selectable,
        int const cost_select,
        int const cost_change,
        std::vector<std::vector<point_type>> const& block
        )
        : problem_id(problem_id), player_id(player_id), size(size), selectable(selectable), cost_select(cost_select), cost_change(cost_change), block(block)
    {
    }

    question_data(question_data&& other)
    {
        *this = std::move(other);
    }
    question_data& operator=(question_data&& other)
    {
        this->problem_id  = other.problem_id;
        this->player_id   = other.player_id;
        this->size        = std::move(other.size);
        this->selectable  = other.selectable;
        this->cost_select = other.cost_select;
        this->cost_change = other.cost_change;
        this->block       = std::move(other.block);
        return *this;
    }

    question_data clone() const
    {
        return question_data{
            problem_id,
            player_id,
            size,
            selectable,
            cost_select,
            cost_change,
            block
        };
    }
};

struct question_raw_data : private boost::noncopyable
{
    std::pair<int,int> split_num; // x * y
    int selectable_num;
    std::pair<int,int> cost; // 選択コスト / 交換コスト
    std::pair<int,int> size; // x * y
    int max_brightness; // 最大輝度
    image_type pixels;
    
    question_raw_data()
    {
    }
    question_raw_data(question_raw_data&& other)
    {
        *this = std::move(other);
    }
    question_raw_data& operator=(question_raw_data&& other)
    {
        this->split_num      = std::move(other.split_num);
        this->selectable_num = other.selectable_num;
        this->cost           = std::move(other.cost);
        this->size           = std::move(other.size);
        this->max_brightness = other.max_brightness;
        this->pixels         = std::move(other.pixels);
        return *this;
    }
};

struct answer_type
{
    enum class action_type{ change, select };

    action_type type;
    point_type position;
    char direction;
};
typedef std::vector<answer_type> answer_list;

template<class T>
struct direction_type
{
    // 所謂CSSなどの順番に記述
    T up;
    T right;
    T down;
    T left;
};

//sort_algorithm
typedef std::vector<std::vector<std::vector<std::vector<direction_type<uint64_t>>>>> compared_type;
typedef std::vector<std::vector<direction_type<point_type>>> adjacent_type;

#endif

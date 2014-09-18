#ifndef RESOLVER_DATA_TYPE_HPP
#define RESOLVER_DATA_TYPE_HPP

#include <cstdint>
#include <cmath>
#include <utility>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <boost/functional/hash/extensions.hpp>
#include <opencv2/core/core.hpp>

enum struct AllDirection { Same, Up, UpperRight, Right, DownerRight, Down, DownerLeft, Left, UpperLeft };

struct point_type
{
    int x;
    int y;

    inline std::string const str() const
    {
        return ( boost::format("(%1%,%2%)") % this->x % this->y ).str();
    }
    inline uint16_t num() const
    {
        return this->x * 16 + this->y;
    }

    friend inline bool operator== (point_type const& lhs, point_type const& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
    friend inline bool operator!= (point_type const& lhs, point_type const& rhs)
    {
        return lhs.x != rhs.x || lhs.y != rhs.y;
    }
    friend inline bool operator< (point_type const& lhs, point_type const& rhs)
    {
    return (lhs.x == rhs.x) ? lhs.y < rhs.y : lhs.x < rhs.x;
    }
    friend inline point_type const operator- (point_type const& lhs, point_type const& rhs)
    {
        return point_type{lhs.x - rhs.x, lhs.y - rhs.y};
    }
    friend inline point_type const operator+ (point_type const& lhs, point_type const& rhs)
    {
        return point_type{lhs.x + rhs.x, lhs.y + rhs.y};
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

    inline point_type const up() const
    {
        return point_type{this->x, this->y - 1};
    }
    inline point_type const right() const
    {
        return point_type{this->x + 1, this->y};
    }
    inline point_type const down() const
    {
        return point_type{this->x, this->y + 1};
    }
    inline point_type const left() const
    {
        return point_type{this->x - 1, this->y};
    }

    inline AllDirection direction(point_type const& point) const
    {
        point_type diff = *this - point;
        if (diff.x < 0) {
            if (diff.y < 0) {
                return AllDirection::DownerRight;
            } else if (diff.y > 0) {
                return AllDirection::UpperRight;
            } else {
                return AllDirection::Right;
            }
        } else if (diff.x > 0) {
            if (diff.y < 0) {
                return AllDirection::DownerLeft;
            } else if (diff.y > 0) {
                return AllDirection::UpperLeft;
            } else {
                return AllDirection::Left;
            }
        } else {
            if (diff.y < 0) {
                return AllDirection::Down;
            } else if (diff.y > 0) {
                return AllDirection::Up;
            } else {
                return AllDirection::Same;
            }
        }
    }

    friend std::size_t hash_value(point_type const& point)
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, point.x);
        boost::hash_combine(seed, point.y);
        return seed;
    }
};

typedef cv::Vec3b                            pixel_type;
typedef std::vector<uint8_t>                 unfold_image_type;
typedef cv::Mat_<cv::Vec3b>                  image_type;

// [i][j]の位置に分割された画像(cv::Mat_<cv::Vec3b>)が入っている．
typedef std::vector<std::vector<image_type>> split_image_type;

struct question_data //: private boost::noncopyable
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

//    question_data(question_data&& other)
//    {
//        *this = std::move(other);
//    }

    question_data()
    {
    }

//    question_data& operator=(question_data&& other)
//    {
//        this->problem_id = other.problem_id;
//        this->player_id = other.player_id;
//        this->size = std::move(other.size);
//        this->selectable = other.selectable;
//        this->cost_select = other.cost_select;
//        this->cost_change = other.cost_change;
//        this->block = std::move(other.block);
//        return *this;
//    }

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
    point_type position;
    std::vector<char> actions;
};
typedef std::vector<answer_type> answer_list;

struct step_type {
    answer_list answer;
    point_type selecting_cur;
    std::vector<std::vector<point_type>> matrix;

    friend bool operator== (step_type const& lhs, step_type const& rhs)
    {
        return lhs.matrix == rhs.matrix;
    }
};

template<class T>
struct direction_type
{
    // 所謂CSSなどの順番に記述
    T up;
    T right;
    T down;
    T left;
};

// ostream に吐けると便利だよね
template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<< (std::basic_ostream<CharT, Traits>& os, point_type const& point)
{
    os << point.str();
    return os;
}

//sort_algorithm
typedef std::vector<std::vector<std::vector<std::vector<direction_type<uint64_t>>>>> compared_type;
typedef std::vector<std::vector<direction_type<point_type>>> adjacent_type;

//yrange2
struct answer_type_y{
	std::vector<std::vector<std::vector<point_type>>> point_type;
	std::vector<double> score;
	std::vector<cv::Mat> cv_Mat;
};
struct cr_set{
	cv::Mat row;
	cv::Mat column;
	std::vector<std::unordered_map<point_type, cv::Mat>> each_direction;
};

namespace std
{
    template <>
    struct hash<step_type>
    {
        std::size_t operator() (step_type const& step) const
        {
            std::size_t result;
            for (auto row : step.matrix) {
                for (auto point : row) {
                    boost::hash_combine(result, point);
                }
            }
            return result;
        }
    };

    template <>
    struct hash<point_type>
    {
        std::size_t operator() (point_type const& point) const
        {
            return hash_value(point);
        }
    };
}

enum direction {
    up, right, down, left
};

#endif

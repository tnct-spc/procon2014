#ifndef RESOLVER_DATA_TYPE_HPP
#define RESOLVER_DATA_TYPE_HPP

#include <cstdint>
#include <cmath>
#include <utility>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
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

    inline AllDirection const direction(point_type const& point) const
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
};

typedef cv::Vec3b                            pixel_type;
typedef std::vector<uint8_t>                 unfold_image_type;
typedef cv::Mat_<cv::Vec3b>                  image_type;

// [i][j]の位置に分割された画像(cv::Mat_<cv::Vec3b>)が入っている．
typedef std::vector<std::vector<image_type>> split_image_type;

struct question_data : private boost::noncopyable
{
    std::pair<int,int> size;
    int selecrtable;
    int cost_select;
    int cost_change;
    std::vector<std::vector<point_type>> block;

    question_data(
        std::pair<int,int> const& size,
        int const selecrtable,
        int const cost_select,
        int const cost_change,
        std::vector<std::vector<point_type>> const& block
        )
        : size(size), selecrtable(selecrtable), cost_select(cost_select), cost_change(cost_change), block(block)
    {
    }

    question_data(question_data&& other)
    {
        *this = std::move(other);
    }
    question_data& operator=(question_data&& other)
    {
        this->size        = std::move(other.size);
        this->selecrtable = other.selecrtable;
        this->cost_select = other.cost_select;
        this->cost_change = other.cost_change;
        this->block       = std::move(other.block);
        return *this;
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

// 斜め方向を表す列挙型
enum struct DiagonalDirection { UpperRight, DownerRight, DownerLeft, UpperLeft };

// 水平垂直方向を表す列挙型
enum struct HVDirection { Up, Right, Down, Left };

// 八方を表す列挙型
//enum struct AllDirection { Same, Up, UpperRight, Right, DownerRight, Down, DownerLeft, Left, UpperLeft };

inline char const direction_char(HVDirection const& d)
{
    return "URDL"[static_cast<int>(d)];
}

struct answer_line
{
    point_type select;
    std::vector<HVDirection> change_list;
};
struct answer_type
{
    std::vector<answer_line> list;

    std::string const& str() const
    {
        static std::string answer_string;

        answer_string.erase(answer_string.begin(), answer_string.end());
        for (auto step : list) {
            answer_string += (boost::format("%1$02X") % step.select.num()).str();
            answer_string.push_back('\n');
            for (auto direction : step.change_list) {
                answer_string.push_back(direction_char(direction));
            }
            answer_string.push_back('\n');
        }

        return answer_string;
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

#endif

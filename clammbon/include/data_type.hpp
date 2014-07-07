#ifndef RESOLVER_DATA_TYPE_HPP
#define RESOLVER_DATA_TYPE_HPP

#include <cstdint>
#include <cmath>
#include <tuple>
#include <utility>
#include <vector>
#include <boost/noncopyable.hpp>

struct point_type
{
    int x;
    int y;

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
};
struct pixel_type
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    friend bool operator== (pixel_type const& lhs, pixel_type const& rhs)
    {
        return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
    }
};

typedef std::vector<uint8_t>                 unfold_image_type;
typedef std::vector<std::vector<pixel_type>> image_type;

// 気持ち悪いが，[i][j]の位置に分割された画像が入っている．更に[j][k]へのアクセスによって画素にアクセス
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

struct answer_type
{
    enum class action_type{ change, select };

    action_type type;
    point_type possition;
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

enum struct TurnSide { UpperRight, DownerRight, DownerLeft, UpperLeft };
enum struct Direction { Up, Right, Down, Left };

// ostream に吐けると便利だよね
template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, point_type const& point)
{
    os << point.x << "," << point.y;
    return os;
}

#endif

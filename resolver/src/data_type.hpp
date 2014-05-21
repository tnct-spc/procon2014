#ifndef RESOLVER_DATA_TYPE_HPP
#define RESOLVER_DATA_TYPE_HPP

#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

struct point_type
{
    int x;
    int y;
};
struct pixel_type
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

typedef std::vector<std::vector<pixel_type>> image_type;

// 気持ち悪いが，[i][j]の位置に分割された画像が入っている．更に[j][k]へのアクセスによって画素にアクセス
typedef std::vector<std::vector<image_type>> split_image_type;

struct question_data
{
    std::pair<int,int> size;
    int selecrtable;
    int cost_select;
    int cost_change;

    std::vector<std::vector<point_type>> block;
};

struct question_raw_data
{
public:
    std::pair<int,int> split_num; // x * y
    int selectable_num;
    std::pair<int,int> cost; // 選択コスト / 交換コスト
    std::pair<int,int> size; // x * y
    int max_brightness; // 最大輝度

    image_type pixels;
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

#endif

#ifndef DATA_TYPE_HPP
#define DATA_TYPE_HPP

#include <utility>
#include <vector>
#include <tuple>

struct question_data
{
    std::pair<int,int> size;
    int selecrtable;
    int cost_select;
    int cost_change;

    std::vector<std::vector<int>> block;
};

struct question_raw_data
{
    std::pair<int,int> split_num; // x * y
    int selectable_num;
    std::pair<int,int> cost; // 選択コスト / 交換コスト
    std::pair<int,int> size; // x * y
    int max_brightness; // 最大輝度

    std::vector<std::vector<std::tuple<char,char,char>>> pixels;
};

struct answer_type
{
    enum class action_type{ change, select };

    action_type type;
    std::pair<int,int> possition;
    char direction;
};

typedef std::vector<answer_type> answer_list;

#endif

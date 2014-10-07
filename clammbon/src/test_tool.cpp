﻿#include <stdexcept>
#include <utility>
#include "test_tool.hpp"

namespace test_tool
{
    emulator::emulator(question_data const& question)
        : question_(question.clone())
    {
    }

    auto emulator::start(answer_type const& answer) -> return_type
    {
        auto const emulated = emulate_movement(answer);
        auto const cost     = count_cost(answer);
        auto const correct  = count_correct(emulated);

        return return_type{
            cost,
            question_.size.first * question_.size.second - correct
        };
    }

    point_type emulator::target_point(char const identifier, point_type const& reference) const
    {
        switch (identifier) {
            case 'U':
                return {reference.x, reference.y - 1};
            case 'D':
                return {reference.x, reference.y + 1};
            case 'L':
                return {reference.x - 1, reference.y};
            case 'R':
                return {reference.x + 1, reference.y};
        }
        throw std::runtime_error("emulator::target_point: Undefined identifier");
    }
    
    auto emulator::create_default() -> locate_type
    {
        locate_type locate(
            question_.size.second,
            std::vector<point_type>(question_.size.first, point_type{-1, -1})
            );

        for(int i = 0; i < question_.size.second; ++i)
        {
            for(int j = 0; j < question_.size.first; ++j)
            {
                locate[i][j] = point_type{j, i};
            }
        }

        return locate;
    }

    auto emulator::emulate_movement(answer_type const& answer) -> locate_type
    {
        locate_type state = create_default();
        point_type selected{-1, -1};

        for(auto const& select : answer.list)
        {
            selected = select.position;
            for(const char action : select.actions)
            {
                // 移動先を見つけて交換(std::vectorからあふれた時はatが例外を送出する)
                auto const target = target_point(action, selected);
                std::swap(
                    state.at(selected.y).at(selected.x),
                    state.at(target  .y).at(target  .x)
                    );

                // 更新
                selected = target;
            }
        }

        return state;
    }

    int emulator::count_cost(answer_type const& answer)
    {
        int cost = 0;

        for(auto const& select : answer.list)
        {
            cost += question_.cost_select;
            for(char const action : select.actions)
            {
                cost += question_.cost_change;
            }
        }

        return cost;
    }

    int emulator::count_correct(locate_type const& locate)
    {
        int correct = 0;
        for(int i = 0; i < locate.size(); ++i)
        {
            for(int j = 0; j < locate.at(i).size(); ++j)
            {
                if(locate.at(i).at(j) == question_.block[i][j]) ++correct;
            }
        }
        return correct;
    }

} // namespace test_tool


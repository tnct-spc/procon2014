#include <stdexcept>
#include <utility>
#include "test_tool.hpp"

namespace test_tool
{
    emulator::emulator(question_data const& question)
        : question_(question.clone())
    {
    }

    auto emulator::start(answer_list const& answer) -> return_type
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

    auto emulator::emulate_movement(answer_list const& answer) -> locate_type
    {
        locate_type state = question_.block;
        point_type selected{-1, -1};

        for(auto const& select : answer)
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

    int emulator::count_cost(answer_list const& answer)
    {
        int cost = 0;

        for(auto const& select : answer)
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
        for(size_t i = 0; i < locate.size(); ++i)
        {
            for(size_t j = 0; j < locate.at(i).size(); ++j)
            {
                if(locate.at(i).at(j) == point_type{j, i}) ++correct;
            }
        }
        return correct;
    }

} // namespace test_tool


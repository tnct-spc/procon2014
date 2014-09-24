#ifndef CLAMMBON_TEST_TOOL_HPP
#define CLAMMBON_TEST_TOOL_HPP

#include <vector>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"

namespace test_tool
{
    class emulator : boost::noncopyable
    {
    public:
        typedef std::vector<std::vector<point_type>> locate_type;

        struct return_type
        {
            int cost;
            int wrong;
        };

        emulator(question_data const& question);
        virtual ~emulator() = default;

        auto start(answer_list const& answer) -> return_type;

    private:
        // 識別子(U,D,R,L)と基準位置(reference)から，対象となる交換先を見つける
        point_type target_point(char const identifier, point_type const& reference) const;

        auto create_default() -> locate_type;

        auto emulate_movement(answer_list const& answer) -> locate_type;
        int count_cost(answer_list const& answer);
        int count_correct(locate_type const& locate);

        question_data const question_;
    };
} // namespace test_tool

#endif


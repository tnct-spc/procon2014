#include <iostream>
#include <array>
#include <vector>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"
#include "pixel_sorter.hpp"
#include "ppm_reader.hpp"
#include "algorithm.hpp"
#include "gui.hpp"

#include <sort_algorithm/yrange2.hpp>
#include <sort_algorithm/genetic.hpp>

class analyzer : boost::noncopyable
{
public:
    explicit analyzer() = default;
    virtual ~analyzer() = default;

    question_data operator() ()
    {
#if 1
        std::string path("prob01.ppm");
#else
        std::string path = download();
#endif

        ppm_reader reader(path);
        auto const raw = reader();

        auto future = gui::make_mansort_window(raw, "test");

        question_data formed = {
            raw.split_num,
            raw.selectable_num,
            raw.cost.first,
            raw.cost.second,
            sorter_(raw)
        };

        auto man_resolved = future.get();

        // TODO: ここで，sorter_(raw)の結果がイマイチなら，
        // man_resolvedが結果を置き換える可能性を考慮．

        return std::move(formed);
    }

private:
    pixel_sorter<yrange2> sorter_;
};

// 問題の並び替えパズル自体は，人間が行うほうがいいかもしれない．

int main()
{
    analyzer analyze;
    auto const data = analyze();

    algorithm algorithm;
    algorithm(data);

    return 0;
}

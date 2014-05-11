#include <iostream>
#include <array>
#include <vector>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"
#include "splitter.hpp"
#include "pixel_sorter.hpp"
#include "ppm_reader.hpp"
#include "algorithm.hpp"


class analyzer : boost::noncopyable
{
public:
    explicit analyzer() = default;
    virtual ~analyzer() = default;

    question_data operator() ()
    {
#if 1
        std::string path("../../procon2014/prob01.ppm");
#else
        std::string path = download();
#endif

        ppm_reader reader(path);
        auto const raw = reader();

        auto const splited_image = split_(raw);
        return sorter_(raw, splited_image);
    }

private:
    splitter split_;
    pixel_sorter sorter_;
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

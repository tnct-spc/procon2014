#include <iostream>
#include <array>
#include <vector>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"
#include "splitter.hpp"
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
        return split(raw);
    }

private:
    splitter split;
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

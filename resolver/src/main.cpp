﻿#include <iostream>
#include <array>
#include <vector>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"
#include "pixel_sorter.hpp"
#include "ppm_reader.hpp"
#include "algorithm.hpp"
#include "network.hpp"

class analyzer : boost::noncopyable
{
public:
    explicit analyzer()
        : netclient_()
    {
    }
    virtual ~analyzer() = default;

    question_data operator() ()
    {
#if 1
        std::string path("prob01.ppm");
#else
        std::string path = netclient_.get_problem(01).get();
#endif

        ppm_reader reader(path);
        auto const raw = reader();
        
        question_data formed = {
            raw.split_num,
            raw.selectable_num,
            raw.cost.first,
            raw.cost.second,
            sorter_(raw)
        };

        return std::move(formed);
    }

private:
    network::client netclient_;
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

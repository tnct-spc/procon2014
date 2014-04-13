// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS

#define BOOST_LIB_DIAGNOSTIC

#include <iostream>
#include <array>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"
#include "splitter.hpp"
#include "algorithm.hpp"

class ppm_reader : boost::noncopyable
{
public:
    explicit ppm_reader(boost::filesystem::path const& filepath)
        : filepath_(filepath)
    {
    }

    virtual ~ppm_reader() = default;

    question_raw_data operator() ()
    {
        if(!boost::filesystem::exists(filepath_))
        {
            std::string const message = "Not found: " + filepath_.string() + "\n";
            std::cout << message << std::flush;
            throw std::runtime_error(message);
        }

        question_raw_data raw_data;

        ifs_.open(filepath_, std::ios::binary || std::ios::in);
        read_header(raw_data);
        read_body(raw_data);
        ifs_.close();

        return raw_data;
    }

private:
    void read_header(question_raw_data& output)
    {
        int state = 0; // state == line_num
        std::string line;
        std::vector<std::string> spilited;
        spilited.reserve(3);

        while(state <= 5)
        {
            std::getline(ifs_, line);

            // header line
            switch(state)
            {
            case 0:
                // PPM type
                if(line != "P6") throw std::runtime_error("Not support type: PPM is not P6.");
                break;

            case 1:
                // 分割数
                boost::algorithm::split(spilited, line, boost::is_any_of(" "));
                output.split_num = std::pair<int,int>(std::stoi(spilited[1]), std::stoi(spilited[2]));
                break;

            case 2:
                // 選択可能回数
                boost::algorithm::split(spilited, line, boost::is_any_of(" "));
                output.selectable_num = std::stoi(spilited[1]);
                break;

            case 3:
                // コスト交換レート
                boost::algorithm::split(spilited, line, boost::is_any_of(" "));
                output.cost = std::pair<int,int>(std::stoi(spilited[1]), std::stoi(spilited[2]));
                break;

            case 4:
                // ピクセル数
                boost::algorithm::split(spilited, line, boost::is_any_of(" "));
                output.size = std::pair<int,int>(std::stoi(spilited[0]), std::stoi(spilited[1]));
                break;

            case 5:
                // 最大輝度
                output.max_brightness = std::stoi(line);
                break;

            default:
                throw std::runtime_error("reached switch default label");
            }

            ++state;
        }

        return;
    }

    void read_body(question_raw_data& output)
    {
        std::vector<char> data(output.size.first * 3);

        for(int i=0; i<output.size.second; ++i)
        {
            ifs_.read(data.data(), output.size.first * 3);

            std::vector<std::tuple<char,char,char>> pixel_line;
            pixel_line.reserve(output.size.first);

            for(int j=0; j<output.size.first; ++j)
                pixel_line.emplace_back(data[j*3+0], data[j*3+1], data[j*3+2]);

            output.pixels.push_back(std::move(pixel_line));
        }

        return;
    }

    boost::filesystem::path const filepath_;
    boost::filesystem::ifstream ifs_;
};

class analyzer : boost::noncopyable
{
public:
    explicit analyzer() = default;
    virtual ~analyzer() = default;

    question_data operator() ()
    {
#if 1
        boost::filesystem::path path("../../procon2014/prob01.ppm");
#else
        boost::filesystem::path path = download();
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

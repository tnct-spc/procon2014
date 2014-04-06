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

class splitter
{
public:
    explicit splitter() = default;
    virtual ~splitter() = default;

    question_data operator() (question_raw_data const& raw)
    {
        question_data formed = {
            raw.split_num,
            raw.selectable_num,
            raw.cost.first,
            raw.cost.second,
            std::vector<std::vector<int>>(raw.split_num.second, std::vector<int>(raw.split_num.first, INT_MAX) )
        };

        auto& block = formed.block;

        //
        // Sub Algorithm
        // 正しい位置に並べた時に左上から，1~nまでの番号をふり，それが今どこにあるのかという情報をblockに格納
        //

        return formed;
    }
};

class analyzer : boost::noncopyable
{
public:
    explicit analyzer() = default;
    virtual ~analyzer() = default;

    question_data operator() ()
    {
#if 1
        boost::filesystem::path path("../../25th-kosen-procon/prob01.ppm");
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

class algorithm
{
public:
    explicit algorithm() = default;
    virtual ~algorithm() = default;

    void operator() (question_data const& data)
    {
        //
        // Main Algorithm
        // 正しい順序に並べてみたり，データ送ったり．
        //
    }
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

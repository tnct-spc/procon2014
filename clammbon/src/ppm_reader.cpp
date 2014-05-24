// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS
#include <boost/algorithm/string.hpp>

#include <boost/noncopyable.hpp>
#include "data_type.hpp"
#include "ppm_reader.hpp"

ppm_reader::ppm_reader(std::string const& filepath)
    : filepath_(filepath)
{
}

ppm_reader::~ppm_reader() = default;

question_raw_data ppm_reader::operator() ()
{
    //if(!boost::filesystem::exists(filepath_))
    //{
    //    std::string const message = "Not found: " + filepath_.string() + "\n";
    //    std::cout << message << std::flush;
    //    throw std::runtime_error(message);
    //}

    question_raw_data raw_data;

    ifs_.open(filepath_, std::ios::binary | std::ios::in);
    if(!ifs_.is_open())
    {
        std::string const message = "Not found: " + filepath_ + "\n";
        throw std::runtime_error(message);
    }

    read_header(raw_data);
    read_body(raw_data);
    ifs_.close();

    return raw_data;
}

void ppm_reader::read_header(question_raw_data& output)
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

void ppm_reader::read_body(question_raw_data& output)
{
    std::vector<uint8_t> data(output.size.first * 3);

    for(int i=0; i<output.size.second; ++i)
    {
        ifs_.read(reinterpret_cast<char*>(data.data()), output.size.first * 3);

        std::vector<pixel_type> pixel_line;
        pixel_line.reserve(output.size.first);

        for(int j=0; j<output.size.first; ++j)
            pixel_line.push_back(pixel_type{data[j*3+0], data[j*3+1], data[j*3+2]});

        output.pixels.push_back(std::move(pixel_line));
    }

    return;
}

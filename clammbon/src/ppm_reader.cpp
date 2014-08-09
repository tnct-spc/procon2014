// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS
#include <boost/algorithm/string.hpp>

#include <iterator>
#include <opencv2/opencv.hpp>
#include "data_type.hpp"
#include "ppm_reader.hpp"

#ifdef _DEBUG
#include <iostream>
#endif

ppm_reader::ppm_reader() = default;
ppm_reader::~ppm_reader() = default;

question_raw_data ppm_reader::from_data(std::string const& data)
{
    question_raw_data out;

    read_header(out, data);
    read_body  (out, data);

    return out;
}

question_raw_data ppm_reader::from_file(std::string const& path)
{
    // pathのオープン
    std::ifstream ifs(path);
    if(!ifs.is_open())
    {
        std::string const message = "Not found: " + path + "\n";
#ifdef _DEBUG
        std::cout << message << std::flush;
#endif
        throw std::runtime_error(message);
    }

    // イテレータで全てをvectorに
    std::string data(
        (std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>())
        );

    // 委託
    return from_data(data);
}

void ppm_reader::read_header(question_raw_data& out, std::string const& src)
{
    int state = 0; // state == line_num
    std::string line;

    std::vector<std::string> spilited;
    spilited.reserve(3);

    std::istringstream iss(src);
    while(state <= 5)
    {
        std::getline(iss, line);

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
            out.split_num = std::pair<int,int>(std::stoi(spilited[1]), std::stoi(spilited[2]));
            break;

        case 2:
            // 選択可能回数
            boost::algorithm::split(spilited, line, boost::is_any_of(" "));
            out.selectable_num = std::stoi(spilited[1]);
            break;

        case 3:
            // コスト交換レート
            boost::algorithm::split(spilited, line, boost::is_any_of(" "));
            out.cost = std::pair<int,int>(std::stoi(spilited[1]), std::stoi(spilited[2]));
            break;

        case 4:
            // ピクセル数
            boost::algorithm::split(spilited, line, boost::is_any_of(" "));
            out.size = std::pair<int,int>(std::stoi(spilited[0]), std::stoi(spilited[1]));
            break;

        case 5:
            // 最大輝度
            out.max_brightness = std::stoi(line);
            break;

        default:
            throw std::runtime_error("reached switch default label");
        }

        ++state;
    }

    return;
}

void ppm_reader::read_body(question_raw_data& out, std::string const& src)
{
    // std::string -> std::vector -> cv::Mat
    std::vector<unsigned char> data(src.cbegin(), src.cend());
    out.pixels = cv::imdecode(cv::Mat(data), 1);
    return;
}


// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS
#include <boost/algorithm/string.hpp>

#include <boost/noncopyable.hpp>
#include <opencv2/highgui/highgui.hpp>
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

    // STLでOpenして，ppmのヘッダ部分のみ読み込む
    ifs_.open(filepath_, std::ios::binary | std::ios::in);
    if(!ifs_.is_open())
    {
        std::string const message = "Not found: " + filepath_ + "\n";
        throw std::runtime_error(message);
    }
    read_header(raw_data);
    ifs_.close();

    // OpenCVによって，cv::Matを受け取る
    read_body(raw_data);

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
    output.pixels = cv::imread(filepath_);
    return;
}

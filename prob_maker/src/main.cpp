#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int constexpr hsplit_default = 16;
int constexpr vsplit_default = 16;
int constexpr selectable_default = 16;
int constexpr select_cost_default = 1;
int constexpr change_cost_default = 1;
std::string const ext = ".ppm";

int main(int argc, char* argv[])
{
    std::vector<std::string> input_filenames;
    std::string output_filename_base;
    int hsplit, vsplit;
    int selectable;
    int select_cost, change_cost;

    // コマンドラインオプションの定義
    boost::program_options::options_description options(argv[0]);
    options.add_options()
        ("input,i",         boost::program_options::value<std::vector<std::string>>(&input_filenames)   ->multitoken() ->required(),            "入力ファイル名")
        ("output,o",        boost::program_options::value<std::string>(&output_filename_base),                                                  "出力ファイル名")
        ("hsplit,h",        boost::program_options::value<int>(&hsplit)                                 ->default_value(hsplit_default),        "横分割数")
        ("vsplit,v",        boost::program_options::value<int>(&vsplit)                                 ->default_value(vsplit_default),        "縦分割数")
        ("selectable,n",    boost::program_options::value<int>(&selectable)                             ->default_value(selectable_default),    "選択可能回数")
        ("select_cost,s",   boost::program_options::value<int>(&select_cost)                            ->default_value(select_cost_default),   "選択コスト変換レート")
        ("change_cost,c",   boost::program_options::value<int>(&change_cost)                            ->default_value(change_cost_default),   "交換コスト変換レート")
    ;

    // コマンドラインオプションのチェック
    boost::program_options::variables_map options_map;
    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), options_map);
        boost::program_options::notify(options_map);
    } catch (boost::program_options::error_with_option_name const& e) {
        std::cout << e.what() << std::endl;
        std::cout << options << std::endl;
        return -1;
    }

    // 入力ファイル名でループ
    cv::Mat input_image;
    std::string header;
    std::vector<uchar> output_buffer;
    std::string output_filename;
    std::ofstream output_file;
    output_file.exceptions(std::ofstream::eofbit | std::ofstream::failbit | std::ofstream::badbit);

    for (std::string const& input_filename : input_filenames) {
        // 出力ファイル名の設定
        if (output_filename_base.size() == 0) {
            output_filename = input_filename;
        } else {
            output_filename = output_filename_base;
        }
        auto dotpos = output_filename.find_last_of('.');
        if (dotpos != std::string::npos) {
            output_filename.resize(dotpos);
        }
        output_filename += ext;

        // ヘッダ文字列の設定
        header.erase();
        header += (boost::format("# %1% %2%\n") % hsplit % vsplit).str();
        header += (boost::format("# %1%\n") % selectable).str();
        header += (boost::format("# %1% %2%\n") % select_cost % change_cost).str();

        // 画像読み込み
        input_image = cv::imread(input_filename);
        if (input_image.data == nullptr) {
            std::cerr << "Fatal: " << input_filename << " を読み込めませんでした" << std::endl;
            return -1;
        }

        // バッファへ書き込み
        cv::imencode(ext, input_image, output_buffer, std::vector<int>(CV_IMWRITE_PXM_BINARY));

        // ヘッダ流し込み
        // 先頭の "P6\n" の後にヘッダ文字列を挿入する
        output_buffer.insert(output_buffer.begin() + 3, header.begin(), header.end());

        // ファイル書き出し
        try {
            output_file.open(output_filename, std::ofstream::binary);
            std::copy(output_buffer.begin(), output_buffer.end(), std::ostream_iterator<uchar>(output_file));
            output_file.close();
        } catch (std::ofstream::failure e) {
            std::cerr << "Fatal: " << output_filename << " へ書き込めません";
            if (output_file.bad()) {
                std::cerr << " (bad)";
            } else if (output_file.fail()) {
                std::cerr << " (fail)";
            } else {
                std::cerr << " (eof)";
            }
            std::cerr << std::endl;
            return -1;
        }
    }
    return 0;
}

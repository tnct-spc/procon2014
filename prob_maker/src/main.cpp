#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <utility>
#include <vector>
#include <cmath>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "data_type.hpp"

int constexpr piece_size_max = 128;
int constexpr piece_size_min = 16;
int constexpr split_max = 16;
int constexpr split_min = 2;
int constexpr image_size_max = 1024;
int constexpr image_size_min = split_min * piece_size_min;

int constexpr selectable_default = 16;
int constexpr select_cost_default = 1;
int constexpr change_cost_default = 1;
int constexpr piece_width_default = -1;
int constexpr piece_height_default = -1;
int constexpr horizontal_split_default = -1;
int constexpr vertical_split_default = -1;

int main(int argc, char* argv[])
{
    std::vector<std::string> input_filenames;  // 入力ファイルの名前の配列
    std::string output_filename_base;  // 出力ファイル名のもと
    cv::Size output_size;  // 出力画像の幅と高さ
    int piece_width, piece_height;  // 断片画像の幅と高さ
    int horizontal_split, vertical_split;  // 分割数
    int selectable;  // 選択可能回数
    int select_cost, change_cost;  // 選択コスト変換レート

    // コマンドラインオプションの定義
    boost::program_options::options_description options(argv[0]);
    options.add_options()
        (",i",  boost::program_options::value<std::vector<std::string>>(&input_filenames)   ->multitoken() ->required(),                "入力ファイル名")
        (",o",  boost::program_options::value<std::string>(&output_filename_base),                                                      "出力ファイル名")
        ("px",  boost::program_options::value<int>(&piece_width)                            ->default_value(piece_width_default),       "断片画像の幅")
        ("py",  boost::program_options::value<int>(&piece_height)                           ->default_value(piece_height_default),      "断片画像の高さ")
        ("sx",  boost::program_options::value<int>(&horizontal_split)                       ->default_value(horizontal_split_default),  "横分割数")
        ("sy",  boost::program_options::value<int>(&vertical_split)                         ->default_value(vertical_split_default),    "縦分割数")
        (",n",  boost::program_options::value<int>(&selectable)                             ->default_value(selectable_default),        "選択可能回数")
        (",s",  boost::program_options::value<int>(&select_cost)                            ->default_value(select_cost_default),       "選択コスト変換レート")
        (",c",  boost::program_options::value<int>(&change_cost)                            ->default_value(change_cost_default),       "交換コスト変換レート")
    ;

    // コマンドラインオプションのチェック
    boost::program_options::variables_map options_map;
    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), options_map);
        boost::program_options::notify(options_map);
    } catch (boost::program_options::error_with_option_name const& e) {
        std::cerr << e.what() << std::endl;
        std::cout << options << std::endl;
        return -1;
    }

    cv::Mat input_image;  // 入力画像
    cv::Mat temp_image;  // 作業場
    cv::Mat output_image;  // 出力画像
    std::string header;  // ヘッダ文字列
    std::stringstream answer_stream;  // .ans ファイルの中身
    std::vector<uchar> output_buffer;  // 出力画像バッファ
    std::string output_filename;  // 出力ファイルの名前
    std::string output_image_filename;  // 出力画像ファイルの名前
    std::string output_answer_filename;  // 出力正答ファイルの名前
    std::ofstream output_file;  // 出力ファイルのストリーム
    cv::Size input_size;  // 入力画像のサイズ
    int count = 0;

    output_file.exceptions(std::ofstream::eofbit | std::ofstream::failbit | std::ofstream::badbit);

    // 入力ファイル名でループ
    for (std::string const& input_filename : input_filenames) {
        // 出力ファイル名の設定
        if (output_filename_base.size() == 0) {
            output_filename = input_filename;
        } else {
            output_filename = output_filename_base;
            if (input_filenames.size() > 1) {
                output_filename += (boost::format("_%1$d") % count).str();
                ++count;
            }
        }
        auto dotpos = output_filename.find_last_of('.');
        if (dotpos != std::string::npos) {
            output_filename.resize(dotpos);
        }
        output_image_filename = output_filename + ".ppm";
        output_answer_filename = output_filename + ".ans";

        // 画像読み込み
        input_image = cv::imread(input_filename);
        if (input_image.data == nullptr) {
            std::cerr << "Fatal: " << input_filename << " を読み込めませんでした" << std::endl;
            return -1;
        }
        input_size.width = input_image.cols;
        input_size.height = input_image.rows;

        // 分割数, 断片画像サイズ, 全体のサイズの決定と縮小切抜
        bool piece_size_valid = piece_size_min <= piece_width && piece_width <= piece_size_max && piece_size_min <= piece_height && piece_height <= piece_size_max;
        bool split_valid = split_min <= horizontal_split && horizontal_split <= split_max && split_min <= vertical_split && vertical_split <= split_max;
        if (piece_size_valid != split_valid) {
            int max_width, max_height;
            if (piece_size_valid) {
                max_width = std::min(split_max * piece_width, image_size_max);
                max_height = std::min(split_max * piece_height, image_size_max);
            } else {
                max_width = std::min(horizontal_split * piece_size_max, image_size_max);
                max_height = std::min(vertical_split * piece_size_max, image_size_max);
            }

            if (max_width < input_size.width && max_height < input_size.height) {
                if (max_width < input_size.width && max_height < input_size.height) {
                    if (input_size.width > input_size.height) {
                        output_size.width = max_width;
                        output_size.height = input_size.height * max_width / input_size.width;
                    } else {
                        output_size.width = input_size.width * max_height / input_size.height;
                        output_size.height = max_height;
                    }
                } else if (max_width < input_size.width) {
                    output_size.width = max_width;
                    output_size.height = input_size.height * max_width / input_size.width;
                } else if (max_height < input_size.height) {
                    output_size.width = input_size.width * max_height / input_size.height;
                    output_size.height = max_height;
                }
                cv::resize(input_image, temp_image, output_size);
            } else {
                output_size = input_size;
                temp_image = input_image;
            }

            if (piece_size_valid) {
                horizontal_split = output_size.width / piece_width;
                vertical_split = output_size.height / piece_height;
                output_size.width -= output_size.width % piece_width;
                output_size.height -= output_size.height % piece_height;
            } else {
                piece_width = output_size.width / horizontal_split;
                piece_height = output_size.height / vertical_split;
                output_size.width -= output_size.width % horizontal_split;
                output_size.height -= output_size.height % vertical_split;
            }
            temp_image = temp_image.colRange(0, output_size.width).rowRange(0, output_size.height);
        } else {
            std::cerr << "Fatal: " << "分割数と断片画像サイズのどちらかを指定してください" << std::endl;
            return -1;
        }

        // 画像切り分け
        std::vector<std::pair<point_type, cv::Mat>> pieces;
        for (int y = 0; y < vertical_split; ++y) {
            for (int x = 0; x < horizontal_split; ++x) {
                pieces.push_back(
                    std::pair<point_type, cv::Mat>(
                        point_type{x, y},
                        temp_image.colRange(x * piece_width, (x + 1) * piece_width).rowRange(y * piece_height, (y + 1) * piece_height)
                    )
                );
            }
        }

        // シャッフル
        std::random_device rand_device;
        std::default_random_engine rand_engine(rand_device());
        std::shuffle(pieces.begin(), pieces.end(), rand_engine);

        // 画像貼り合わせと .ans ファイル文字列の作成
        output_image = cv::Mat(output_size, input_image.type());
        answer_stream.str("");
        int index;
        for (int y = 0, i = 0; y < vertical_split; ++y) {
            for (int x = 0; x < horizontal_split; ++x, ++i) {
                pieces[i].second.copyTo(output_image(cv::Rect(x * piece_width, y * piece_height, piece_width, piece_height)));
                auto it = std::find_if(pieces.begin(), pieces.end(), [&x, &y](auto& piece){ return piece.first.x == x && piece.first.y == y; });
                index = static_cast<int>(std::distance(pieces.begin(), it));
                answer_stream << boost::format("(%1%,%2%) ") % (index % horizontal_split) % (index / horizontal_split);
            }
            answer_stream << std::endl;
        }

        // ヘッダ文字列の設定
        header.erase();
        header += (boost::format("# %1% %2%\n") % horizontal_split % vertical_split).str();
        header += (boost::format("# %1%\n") % selectable).str();
        header += (boost::format("# %1% %2%\n") % select_cost % change_cost).str();

        // バッファへ書き込み
        cv::imencode(".ppm", output_image, output_buffer, std::vector<int>(CV_IMWRITE_PXM_BINARY));

        // ヘッダ流し込み
        // 先頭の "P6\n" の後にヘッダ文字列を挿入する
        output_buffer.insert(output_buffer.begin() + 3, header.begin(), header.end());

        // ファイル書き出し
        try {
            output_filename = output_image_filename;
            output_file.open(output_filename, std::ofstream::binary);
            std::copy(output_buffer.begin(), output_buffer.end(), std::ostream_iterator<uchar>(output_file));
            output_file.close();

            output_filename = output_answer_filename;
            output_file.open(output_filename, std::ofstream::out);
            output_file << answer_stream.str();
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

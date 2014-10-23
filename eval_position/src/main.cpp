#include <cstdlib>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/thread.hpp>
#include "ppm_reader.hpp"
#include "image_comparator.hpp"
#include "splitter.hpp"
#include "sort_algorithm/yrange2.hpp"
#include "sort_algorithm/yrange5.hpp"
#include "sort_algorithm/Murakami.hpp"

namespace fs = boost::filesystem;
namespace qi = boost::spirit::qi;

BOOST_FUSION_ADAPT_STRUCT(
    point_type,
    (int, x)
    (int, y)
)

class csv_manager : boost::noncopyable
{
public:
    csv_manager(fs::path const& csv_path)
        : ofs_(csv_path, std::ios_base::binary | std::ios_base::out), all_correct_(0), push_num_(0)
    {
        if(!ofs_.is_open())
        {
            // 副作用が嫌なんだけど
            std::cout << "Cannot open: " << csv_path << std::endl;
        }
        else
        {
            ofs_ << "問題セット,yrange2[0],yrange5[0],Murakami,正答率\r\n";
        }
        return;
    }

    virtual ~csv_manager()
    {
        ofs_ << ",,,平均," << ((double)all_correct_ / (push_num_ * 3) * 100) << "%\r\n";
    }

    // ファイルに書き込むわけだが，trueが帰れば正常書き込み
    bool push(std::string const& identifier, bool const yrange2, bool const yrange5, bool const murakami)
    {
        // openされてないなら帰れ
        if(!ofs_.is_open()) return false;

        int correct = 0;
        if(yrange2 ) ++correct;
        if(yrange5 ) ++correct;
        if(murakami) ++correct;

        ofs_ << boost::format("%s,%s,%s,%s,%f%%\r\n")
            % identifier
            % (yrange2 ? "○" : "×")
            % (yrange5 ? "○" : "×")
            % (murakami ? "○" : "×")
            % ((double)correct / 3 * 100)
            ;

        all_correct_ += correct;
        ++push_num_;

        return true;
    }

private:
    int all_correct_;
    int push_num_;
    fs::ofstream ofs_;
};

boost::optional<std::vector<std::vector<point_type>>> read_answer(fs::path const& path, int const width, int const height)
{
    std::vector<std::vector<point_type>> ret;
    ret.reserve(height);

    fs::ifstream ifs(path);

    for(int i=0; i<height; ++i)
    {
        std::vector<point_type> line;
        line.reserve(width);

        std::string tmp;
        std::getline(ifs, tmp);

        qi::rule<std::string::iterator, point_type(), qi::space_type> const pair_rule
            = qi::lit("(") >> qi::int_ >> qi::lit(",") >> qi::int_ >> qi::lit(")");

        qi::rule<std::string::iterator, std::vector<point_type>(), qi::space_type> const rule
            = +(pair_rule);

        bool success = qi::phrase_parse(
            tmp.begin(), tmp.end(),
            rule,
            qi::space,
            line
            );

        if(!success || line.size()!=width)
        {
            return boost::none;
        }

        ret.push_back(std::move(line));
    }

    return ret;
}

namespace clammbon{
    void exit(int const status)
    {
        // Visual Studioが実装してないから
#ifdef _MSC_VER
        std::exit(status);
#else
        std::quick_exit(status);
#endif
    }
}

int main(int argc, char *argv[])
{
    ppm_reader       reader;
    splitter         sp;
    image_comparator comparator;
    csv_manager      csv("report.csv");

    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        clammbon::exit(1);
    }

    fs::path const directory_path(argv[1]);
    if(!fs::exists(directory_path))
    {
        std::cout << "Not found: " << directory_path << std::endl;
        clammbon::exit(2);
    }

    // 指定ディレクトリ内のファイルをループ
    BOOST_FOREACH(
        fs::path const& filepath,
        std::make_pair(fs::directory_iterator(directory_path), fs::directory_iterator())
        )
    {
        // ppmファイルに対してのみの操作を行うため
        if(filepath.extension() != ".ppm") continue;

        // answerファイルのファイルパスを確認し，存在しなければ検証のパス
        fs::path answer_filename = filepath;
        answer_filename.replace_extension(".ans");
        if(!fs::exists(answer_filename))
        {
            std::cout << "Not found: Answerfile: " << answer_filename << std::endl;
            continue;
        }

        //
        // Start: 問題を解く
        //
        auto const raw_question = reader.from_file(filepath.string());
        auto const split        = sp.split_image(raw_question);
        auto const compared     = comparator.image_comp(raw_question, split);
        
        // ソルバの準備
        yrange2  y2(raw_question, compared);
        yrange5  y5(raw_question, compared);
        Murakami mu(raw_question, compared, true);

        // ソルバ解答の受け皿
        std::vector<answer_type_y> ans_yrange2;
        std::vector<answer_type_y> ans_yrange5;
        std::vector<answer_type_y> ans_murakami;
        
        // yrange2/5の実行
        boost::thread y_thread(
            [&y2, &y5, &ans_yrange2, &ans_yrange5]() -> void
            {
                ans_yrange2 = y2();
                ans_yrange5 = y5(y2.sorted_matrix());
            });

        // Murakamiの実行
        boost::thread m_thread(
            [&mu, &ans_murakami]() -> void
            {
                ans_murakami = mu();
            });

        y_thread.join();
        m_thread.join();
        
        //
        // End: 問題を解く
        //

        //
        // 検証
        //

        // 解答ファイルを読み取る
        auto const correct_answer = read_answer(answer_filename, raw_question.split_num.first, raw_question.split_num.second);
        if(!correct_answer)
        {
            std::cout << "Ignore file: " << answer_filename << std::endl;
            continue;
        }

        // yrange*, Murakamiがそれぞれ正しい答えを持っているかどうか判定
        csv.push(
            filepath.stem().string(),
            !ans_yrange2.empty()  && ans_yrange2[0].points == *correct_answer,
            !ans_yrange5.empty()  && ans_yrange5[0].points == *correct_answer,
            !ans_murakami.empty() && ans_murakami[0].points == *correct_answer
            );
    }

    return 0;
}


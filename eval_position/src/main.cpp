#include <cstdlib>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include "ppm_reader.hpp"
#include "pixel_sorter.hpp"
#include "sort_algorithm/yrange2.hpp"

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
        : ofs_(csv_path, std::ios_base::binary | std::ios_base::out)
    {
        if(!ofs_.is_open())
        {
            // 副作用が嫌なんだけど
            std::cout << "Cannot open: " << csv_path << std::endl;
        }
        else
        {
            ofs_ << "問題セット,解答候補総数,正解総数,不正解総数,正答率\r\n";
        }
        return;
    }

    virtual ~csv_manager() = default;

    // ファイルに書き込むわけだが，trueが帰れば正常書き込み
    bool push(std::string const& identifier, int const total, int const correct)
    {
        // openされてないなら帰れ
        if(!ofs_.is_open()) return false;

        int const failure = total - correct;
        ofs_ << boost::format("%s,%d,%d,%d,%f%%\r\n")
            % identifier % total % correct % failure
            % ((double)correct / total * 100);

        return true;
    }

private:
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
    ppm_reader reader;
    pixel_sorter<yrange2> sorter;
    csv_manager csv("report.csv");

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

        // 問題を解く
        auto const raw_question = reader.from_file(filepath.string());
        auto const proposed_answers = sorter.proposed_answer(raw_question); // TODO: sorterはただ1つの解答を返すべき

        // 解答ファイルを読み取る
        auto const correct_answer = read_answer(answer_filename, raw_question.split_num.first, raw_question.split_num.second);
        if(!correct_answer)
        {
            std::cout << "Ignore file: " << answer_filename << std::endl;
            continue;
        }

        // 正しいものの数を数える
        int correct = 0;
        for(int i=0; i<proposed_answers.size(); ++i)
            if(proposed_answers[i].points == *correct_answer)
                ++correct;

        csv.push(filepath.stem().string(), proposed_answers.size(), correct);
    }

    return 0;
}


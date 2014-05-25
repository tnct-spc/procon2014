#include <fstream>
#include <boost/format.hpp>
#include "ppm_reader.hpp"
#include "pixel_sorter.hpp"

int main(int argc, char *argv[])
{
    pixel_sorter sorter;
    std::ofstream ofs("test.csv");

    for(int n=1; n<argc; ++n)
    {
        ppm_reader reader(argv[n]);
        auto const raw = reader();
        auto const answers = sorter.proposed_answer(raw);

        int correct = 0;
        for(auto const& ans : answers)
        {
            for(int i=0; i<ans.size(); ++i)
            {
                for(int j=0; j<ans.at(0).size(); ++j)
                {
                    if(ans[i][j] == point_type{j, i}) ++correct;
                }
            }
        }

        int success = 0;
        for(auto const& ans : answers)
        {
            for(int i=0; i<ans.size(); ++i)
            {
                for(int j=0; j<ans.at(0).size(); ++j)
                {
                    if(ans[i][j] == point_type{j, i}) goto OUT;
                }
            }
            ++success;
OUT:        ;
        }

        int const failure = answers.size() - success;
        ofs << 
            boost::format("%s,%d,%d,%d,%d,%d\n")
            % argv[n] % answers.size() % success % failure % correct 
            % (answers.size() * answers.at(0).size() * answers.at(0).at(0).size());

        //
        // とりあえず，ここでcsv評価する．
        //
    }


//
//	failure = outputnum - success;
//	std::ofstream ofs("solusions.csv", std::ios::out | std::ios::app | std::ios::ate);
//	ofs << usefile << "," << outputnum << "," << success << "," << failure << "," << correct << "," << outputnum*sepx*sepy << "," << std::endl;
//	//ファイル名,出力された解答の数，完全一致した解答の数，そうでない解答の数，一致したピースの数，全体のピースの数
//#endif
//	std::cout << "Output solusions done." << std::endl;

    return 0;
}


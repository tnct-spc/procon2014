#ifndef CLAMMBON_SORT_GENETIC_HPP
#define CLAMMBON_SORT_GENETIC_HPP

#include <algorithm>
#include <iomanip> // std::setwのみ
#include <boost/range/algorithm.hpp>

int constexpr PARENT = 10;
int constexpr CHILDREN = 100;

class genetic
{
private:
    typedef std::vector<std::vector<std::vector<std::vector<direction_type<uint64_t>>>>> compared_type;
    
	struct children_t { std::vector<int> children_gene; unsigned int assessment;};//子供の遺伝子と評価値
	std::vector<children_t> children_;//子供の遺伝子と評価値の配列
    
    question_raw_data *data_;
    compared_type *comp_;
    
    void shuffle(std::vector<int>& arr) const
    {
        for(int i = 0; i < arr.size(); ++i)
        {
            int j = 0; //TODO: メルセンヌでarr.size()までの値を入れる
            std::swap(arr[i], arr[j]);
        }
    }

    void shuffle_all()
    {
        for(auto it = children_.begin(); it != children_.end(); ++it)
        {
            shuffle(it->children_gene);
        }
    }

    unsigned int assess(std::vector<int> const& arr) const
    {
        auto const sepx = data_->split_num.first;
        auto const sepy = data_->split_num.second;

        unsigned int ass = 0;
        for(int i = 0; i < arr.size(); ++i)
        {
            if(i + 1 < arr.size())
            {
                if(arr[i] == arr[i + 1])
                {
                    std::cout << "!!!" << arr[i] << arr[i + 1] << std::endl;
                }
                ass += (*comp_)[arr[i] / sepx][arr[i] % sepx][arr[i + 1] / sepx][arr[i + 1] % sepx].right;
            }
            if(i + sepy < arr.size())
            {
                if(arr[i] == arr[i + sepy])
                {
                    std::cout << "???" << arr[i] << arr[i + sepy] << std::endl;
                }
                ass += (*comp_)[arr[i] / sepx][arr[i] % sepx][arr[i + sepy] / sepx][arr[i + sepy] % sepx].right;
            }
        }
        return ass;
    }

    void assess_all()
    {
        for(auto it = children_.begin(); it != children_.end(); ++it)
        {
            //for(int i = 0; i < split_total; ++i)
            //{
            //    std::cout << it->children_gene[i] << " - ";
            //}
            it->assessment = assess(it->children_gene);
		    //std::cout << "     " << it->assessment << std::endl;
        }
    }

    std::vector<std::vector<int>> select_parent(std::vector<int>& top10)
    {
        top10.assign(PARENT, 0);

        std::vector<int> childred_copy;
        childred_copy.reserve(children_.size());

        boost::transform(
            children_, std::back_inserter(childred_copy),
            [](children_t const& base){ return base.assessment; }
            );
        boost::sort(childred_copy);
        //boost::for_each(childred_copy, [](int const base){ std::cout << base << std::endl; });

        for(int i=0; i<top10.size(); ++i)
        {
            for(int j=0; j<CHILDREN; ++j)
            {
                if(childred_copy.at(i) == children_[j].assessment)
                {
                    top10[i] = j;
                }
            }
        }

        auto const split_total = data_->split_num.first * data_->split_num.second;
        std::vector<std::vector<int>> parent_gene(PARENT, std::vector<int>(split_total, 0));

        //親遺伝子配列にいいやつTop10を突っ込む
        for(int i=0; i<top10.size(); ++i)
        {
            for(int j=0; j<split_total; ++j)
            {
                parent_gene[i][j] = children_[top10[i]].children_gene[j];
            }
        }

        std::cout << "番号  " << "評価" << std::endl;
        for(auto const& elem : top10)
        {
            std::cout << elem << "   " << children_[elem].assessment << std::endl;
        }

        return parent_gene;
    }


public:
    int sort()
    {
        auto const split_total = data_->split_num.first * data_->split_num.second;

        // children_の初期化
        {
            //連番リスト(0~split_total-1)の作製
            std::vector<int> tmp_vector;
            tmp_vector.reserve(split_total);        
            for(int i = 0; i < split_total; ++i) tmp_vector.push_back(i);

            //連番リストでchildren_を初期化
            children_.assign(CHILDREN, {tmp_vector, 0});
        }

        shuffle_all();
        assess_all();
        
        int top;
        std::vector<int> top10(PARENT);
        std::vector<int> toppers(1000);
        for(int i = 0; i < toppers.size(); ++i)
        {
            for(int j = 0; j < 10000; ++j)
            {
                auto const parent_gene = select_parent(top10);
                top = children_.at(top10[0]).assessment;

                cross_over();
                assess_all();
                
			    std::cout << "最上位遺伝子";
			    for (int i = 0; i < split_total; i++)
                {
				    std::cout << std::setw(3) << parent_gene[0][i]; //unspecified(std::setw)
			    }
                
                std::cout << "番号" << top10[0] << std::endl;

                int const base = top10[0]; // VCのための苦肉の策
                if(
                    std::all_of(
                        top10.begin(), top10.end(),
                        [base](int const elem){ return base == elem; }
                        //[base = top10[0]](int const elem){ return base == elem; }
                    )
                )
                {
                    //Top10の遺伝子がすべて同じものになったら収束したと判断する
                    break;
                }
                std::cout << "世代=" << j << "ループ" << i << std::endl;

            }
            toppers[i] = top;

            shuffle_all();
        }

        
	    int toptop = std::numeric_limits<int>::max();
	    int topnum = 0;
	    for(auto const elem : toppers)
        {
            if(elem < toptop)
            {
                toptop = elem;
                topnum = 0;
            }
            else if(elem == toptop)
            {
                ++topnum;
            }
		    std::cout << elem << std::endl;
        }
	    std::cout << "最適値" << toptop << "数" << topnum << std::endl;
    }

};

#endif

#ifndef CLAMMBON_SORT_GENETIC_HPP
#define CLAMMBON_SORT_GENETIC_HPP

#include <iostream>
#include <algorithm>
#include <functional>
#include <iomanip> // std::setwのみ
#include <random>
#include <boost/range/algorithm.hpp>

int constexpr PARENT_NUM = 10;
int constexpr CHILDREN_NUM = 100;

class genetic
{
private:
    typedef std::vector<std::vector<std::vector<std::vector<direction_type<uint64_t>>>>> compared_type;
    
	struct children_t { std::vector<int> children_gene; unsigned int assessment;};//子供の遺伝子と評価値
	std::vector<children_t> children_;//子供の遺伝子と評価値の配列
    
    question_raw_data *data_;
    compared_type *comp_;

    // std::mt19937_64ほど高級なもの要る？
    mutable std::mt19937 mt_;
    
    void shuffle(std::vector<int>& arr) const
    {
        std::uniform_int_distribution<int> distribution(0, arr.size()-1);

        for(int i = 0; i < arr.size(); ++i)
        {
            int j = distribution(mt_);
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
        top10.assign(PARENT_NUM, 0);

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
            for(int j=0; j<CHILDREN_NUM; ++j)
            {
                if(childred_copy.at(i) == children_[j].assessment)
                {
                    top10[i] = j;
                }
            }
        }

        auto const split_total = data_->split_num.first * data_->split_num.second;
        std::vector<std::vector<int>> parent_gene(PARENT_NUM, std::vector<int>(split_total, 0));

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

    void cross_over(std::vector<std::vector<int>> const& parent_gene)
    {
        auto const split_width  = data_->split_num.first;
        auto const split_height = data_->split_num.second;
        auto const split_total  = split_width * split_height;

        std::uniform_int_distribution<int> parent_dist(0, PARENT_NUM - 1);
        std::uniform_int_distribution<int> mutate_dist(0, 4 - 1);
        std::uniform_int_distribution<int> split_dist(0, split_total - 1);

        for(int i=0; i<CHILDREN_NUM; i+=2)
        {
            auto const& pra1 = parent_gene[parent_dist(mt_)];
            auto const& pra2 = parent_gene[parent_dist(mt_)];
            auto      & child1 = children_[i  ].children_gene;
            auto      & child2 = children_[i+1].children_gene;
            
            //std::cout << "par1 ";
            //boost::for_each(pra1, [](int const elem){ std::cout << elem << "*"; });
            //std::cout << std::endl;
            //
            //std::cout << "par2 ";
            //boost::for_each(pra2, [](int const elem){ std::cout << elem << "*"; });
            //std::cout << std::endl;

            child1.assign(child1.size(), -1);
            child2.assign(child2.size(), -1);

            int select_gene = split_dist(mt_);
            //std::cout << "  s" << select_gene;
            while(child1[select_gene] != pra1[select_gene])
            {
			    child1[select_gene] = pra1[select_gene];
			    child2[select_gene] = pra2[select_gene];

                for(int i=0; i<split_total; ++i)
                {
                    if(child2[select_gene] == pra1[i])
                    {
                        select_gene = i;
                    }
                }
                //std::cout << "  s" << select_gene;
            }

            for(int i=0; i<split_total; ++i)
            {
                if(child1[i] == -1)
                {
				    child1[i] = pra2[i];
				    child2[i] = pra1[i];
                }
            }

            for(int n = parent_dist(mt_); n < 10; ++n) //このループ数をいじらないと収束しなかったりすぐに収束してしまったり
            {
                if(mutate_dist(mt_) == 0) //ここの確率で突然変異率を変化させる
                {
                    int rand1 = split_dist(mt_);
                    int rand2 = split_dist(mt_);
                    std::swap(child1[rand1], child1[rand2]);
                    //std::cout << "rand1" << std::endl;
                }
                if(mutate_dist(mt_) == 0) //ここの確率で突然変異率を変化させる
                {
                    int rand1 = split_dist(mt_);
                    int rand2 = split_dist(mt_);
                    std::swap(child2[rand1], child2[rand2]);
				    //std::cout << "rand2" << std::endl;
                }
            }
            
            //std::cout << std::endl;
            //std::cout << "chi1 ";
            //boost::for_each(child1, [](int const elem){ std::cout << elem << "+"; });
            //
            //std::cout << std::endl;
            //std::cout << "chi2 ";
            //boost::for_each(child2, [](int const elem){ std::cout << elem << "+"; });
            //
            //std::cout << std::endl;
        }
    }

public:
    // 泣きのポインタ渡し
    genetic(question_raw_data *data, compared_type *comp)
        : data_(data), comp_(comp)
    {
        //メルセンヌ・ツイスタの初期化
        std::random_device rd;
        std::vector<std::uint_least64_t> random_seed(10);
        boost::generate(random_seed, std::ref(rd));

        std::seed_seq seed(random_seed.cbegin(), random_seed.cend());
        mt_.seed(seed);
    }
    virtual ~genetic() = default;

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
            children_.assign(CHILDREN_NUM, {tmp_vector, 0});
        }

        shuffle_all();
        assess_all();
        
        int top;
        std::vector<int> top10(PARENT_NUM);
        std::vector<int> toppers(1000);
        for(int i = 0; i < toppers.size(); ++i)
        {
            for(int j = 0; j < 10000; ++j)
            {
                auto const parent_gene = select_parent(top10);
                top = children_.at(top10[0]).assessment;

                cross_over(parent_gene);
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

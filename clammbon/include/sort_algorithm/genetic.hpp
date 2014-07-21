#ifndef CLAMMBON_SORT_GENETIC_HPP
#define CLAMMBON_SORT_GENETIC_HPP

#include <iomanip> // std::setwのみ
#include <random>

int constexpr PARENT_NUM = 10;
int constexpr CHILDREN_NUM = 100;

class genetic
{
private:    
	struct children_t { std::vector<int> children_gene; unsigned int assessment;};//子供の遺伝子と評価値
	std::vector<children_t> children_;//子供の遺伝子と評価値の配列
    
    question_raw_data const& data_;
    compared_type const& comp_;

    // std::mt19937_64ほど高級なもの要る？
    mutable std::mt19937 mt_;
    
    void shuffle(std::vector<int>& arr) const;
    void shuffle_all();

    unsigned int assess(std::vector<int> const& arr) const;
    void assess_all();

    std::vector<std::vector<int>> select_parent(std::vector<int>& top10);
    void cross_over(std::vector<std::vector<int>> const& parent_gene);

public:
    // 泣きのポインタ渡し
    genetic(question_raw_data const& data, compared_type const& comp);
    virtual ~genetic() = default;

    std::vector<std::vector<std::vector<point_type>>> operator()();

};

#endif

#include "data_type.hpp"

class splitter
{
public:
    explicit splitter() = default;
    virtual ~splitter() = default;

    question_data operator() (question_raw_data const& raw) const;

private:
    //nまでの和を求める関数
    inline int n_sum(int n) const;

    //イメージ分割関数
    split_image_type split_image(question_raw_data const& raw) const;


};

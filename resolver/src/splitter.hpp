#include "data_type.hpp"

class splitter
{
public:
    explicit splitter() = default;
    virtual ~splitter() = default;

    inline split_image_type operator() (question_raw_data const& raw) const
    {
        return split_image(raw);
    }

private:
    //イメージ分割関数
    split_image_type split_image(question_raw_data const& raw) const;
};

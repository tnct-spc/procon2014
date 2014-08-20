#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>
#include "algorithm.hpp"

auto algorithm::get() -> boost::optional<return_type>
{
    if(co_ && data_)
    {
        auto&& result = co_.get();
        co_();
        return result;
    }
    return boost::none;
}

void algorithm::reset(question_data const& data)
{
    data_ = data.clone();
    co_   = boost::coroutines::coroutine<return_type>::pull_type(
                boost::bind(&algorithm::process, this, _1)
                );
}

void algorithm::operator() (boost::coroutines::coroutine<return_type>::push_type& yield)
{
    //
    // Main Algorithm
    // 正しい順序に並べてみたり，データ送ったり．
    //
    // TODO: "yield(return_type型の解答)"とすれば，呼び出し元に制御が戻り，送信処理を行える．
    //       呼び出し元が再度algorithm::getを呼びだせば，中断されたところから実行される(ハズ)．
}

void algorithm::process(boost::coroutines::coroutine<return_type>::push_type& yield)
{
    // 訳ありで転送するだけの関数
    (*this)(yield);
}


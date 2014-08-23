#include <boost/bind.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "algorithm.hpp"

class algorithm::impl : boost::noncopyable
{
public:
    typedef algorithm::return_type return_type;

    impl() = default;
    virtual ~impl() = default;

    auto get() -> boost::optional<return_type>;
    void reset(question_data const& data);

private:
    void process(boost::coroutines::coroutine<return_type>::push_type& yield);
    void operator() (boost::coroutines::coroutine<return_type>::push_type& yield);

    boost::optional<question_data> data_;
    boost::coroutines::coroutine<return_type>::pull_type co_;
};

algorithm::algorithm()
    : pimpl_(new impl())
{
}

algorithm::~algorithm()
{
    delete pimpl_;
}

auto algorithm::get() -> boost::optional<return_type>
{
    return pimpl_->get();
}

void algorithm::reset(question_data const& data)
{
    pimpl_->reset(data);
}

auto algorithm::impl::get() -> boost::optional<return_type>
{
    if(co_ && data_)
    {
        auto&& result = co_.get();
        co_();
        return result;
    }
    return boost::none;
}

void algorithm::impl::reset(question_data const& data)
{
    data_ = data.clone();
    co_   = boost::coroutines::coroutine<return_type>::pull_type(
                boost::bind(&impl::process, this, _1)
                );
}

void algorithm::impl::operator() (boost::coroutines::coroutine<return_type>::push_type& yield)
{
    //
    // Main Algorithm
    // 正しい順序に並べてみたり，データ送ったり．
    //
    // TODO: "yield(return_type型の解答)"とすれば，呼び出し元に制御が戻り，送信処理を行える．
    //       呼び出し元が再度algorithm::getを呼びだせば，中断されたところから実行される(ハズ)．
}

void algorithm::impl::process(boost::coroutines::coroutine<return_type>::push_type& yield)
{
    // 訳ありで転送するだけの関数
    (*this)(yield);
}

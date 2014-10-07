#ifndef SERVER_ANSWER_HPP
#define SERVER_ANSWER_HPP

#include "server.hpp"

class Answer
{
private:
    std::ostringstream outerr;
    answer_type final_answer;
    void convert(std::string const& s);
    bool sane;
public:
    Answer(std::string s) : sane(false)
    {
        convert(s);
    }
//    answer()
//    {
//        sane = false;
//    }
    inline std::string get_error()
    {
        return std::move(outerr.str());
    }
    inline answer_type const& get()
    {
        return final_answer;
    }
    inline bool valid()
    {
        return sane;
    }
};

#endif


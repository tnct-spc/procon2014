#ifndef SERVER_ANSWER_HPP
#define SERVER_ANSWER_HPP

#include "server.hpp"

class answer
{
private:
    std::ostringstream outerr;
    answer_list final_answer;
    void convert(std::string const& s);
    bool sane;
public:
    answer(std::string s)
    {
        convert(s);
        sane = false;
    }
//    answer()
//    {
//        sane = false;
//    }
    inline std::string get_error()
    {
        return outerr.str();
    }
    inline answer_list const& get()
    {
        return final_answer;
    }
    inline bool valid()
    {
        return sane;
    }
};

#endif


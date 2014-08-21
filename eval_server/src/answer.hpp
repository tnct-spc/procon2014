#ifndef SERVER_ANSWER_HPP
#define SERVER_ANSWER_HPP

#include "server.hpp"

class answer
{
private:
    std::ostringstream outerr;
    answer_list final_answer;
    answer_list const convert(std::string const& s);
public:
    answer(std::string s)
    {
        final_answer = convert(s);
    }
    answer()
    {}
    inline std::string get_error()
    {
        return outerr.str();
    }
    inline answer_list const& get()
    {
        return final_answer;
    }
};

#endif


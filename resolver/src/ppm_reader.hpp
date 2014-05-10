#ifndef PPM_READER_HPP
#define PPM_READER_HPP

#include <fstream>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"

class ppm_reader : boost::noncopyable
{
public:
    explicit ppm_reader(std::string const& filepath);
    virtual ~ppm_reader();

    question_raw_data operator() ();

private:
    void read_header(question_raw_data& output);
    void read_body(question_raw_data& output);

    std::string const filepath_;
    std::ifstream ifs_;
};

#endif

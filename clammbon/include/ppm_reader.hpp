#ifndef RESOLVER_PPM_READER_HPP
#define RESOLVER_PPM_READER_HPP

#include <fstream>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "data_type.hpp"

class ppm_reader : boost::noncopyable {
public:
    explicit ppm_reader();
    virtual ~ppm_reader();

	question_raw_data from_data(std::string const& data);
    question_raw_data from_file(std::string const& path);

private:
    void read_header(question_raw_data& out, std::string const& src);
    void read_body(question_raw_data& out, std::string const& src);
};

#endif


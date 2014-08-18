// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS
#include <boost/network/protocol/http/server.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include "server.hpp"
namespace http = boost::network::http;

/*<< Defines the server. >>*/
struct submit_form;
typedef http::server<submit_form> server;

struct submit_form {
    void operator() (server::request const &request, server::response &response)
    {
        std::string p = (std::string)destination(request);
        std::string b = (std::string)body(request);
        pcserver pcs;
        if(p == "/SubmitForm") {
            std::ifstream ifs("res/SubmitForm.html");
            std::stringstream buf;
            buf << ifs.rdbuf();
            response = server::response::stock_reply(server::response::ok, buf.str());
        } else if(p == "/SubmitAnswer") {
            auto req = pcs.post_req_to_map(b);
            std::string s_res;
            pcs.parse(pcs.load_problem(req["problemid"]), pcs.string_to_answer(req["answer"]));
            if(req["options"].find("quiet") != std::string::npos) // 見つかった場合
                s_res = pcs.output.str();
            else 
                s_res = pcs.outerr.str() + pcs.output.str();
            response = server::response::stock_reply(server::response::ok, s_res);
        } else if(p.substr(0, 9) == "/problem/") {
            std::ifstream ifs("problem/" + p.substr(9)); // insecure
            if(ifs.fail()) {
                response = server::response::stock_reply(server::response::not_found, "Not found");
            } else {
                response = server::response::stock_reply(server::response::ok, "data");
//                response << boost::network::header("Content-Type", "image/x-portable-pixmap");
//                response << boost::network::header("Connection", "close");
            }
        } else {
            response = server::response::stock_reply((boost::network::http::basic_response<boost::network::http::tags::http_server>::status_type)418, "I'm a tea pot");
        }
    }
    void log(...){}
};

void http_run()
{
    try {
        /*<< Creates the request handler. >>*/
        submit_form handler;
        /*<< Creates the server. >>*/
        server::options options(handler);
        server server_(options.address("127.0.0.1").port("80"));
        /*<< Runs the server. >>*/
        server_.run();
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

int main()
{
    http_run();
    return 0;
}


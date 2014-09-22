#include <signal.h>
#include <mongoose/Server.h>
#include <mongoose/Controller.h>
#include <mongoose/Utils.h>

#include <sstream>
#include <iomanip>

#include "server.hpp"
#include "tools.hpp"
#include "problem.hpp"
#include "answer.hpp"

using namespace Mongoose;

class MyController : public Controller
{
    std::string problem_set = "default";
public:
    void submit_form(Request &req, StreamResponse &res)
    {
        std::ifstream ifs("res/SubmitForm.html");
        res << ifs.rdbuf();
        res.setHeader("Content-Type", "text/html");
    }
    void submit_answer(Request &req, StreamResponse &res)
    {
        std::string const playerid = req.get("playerid", "");
        std::string const problemid = req.get("problemid", "");
        std::string const options = req.get("options", "");
        std::string const answer = req.get("answer", "");

        std::cerr << "playerid: " << playerid << std::endl;
        std::cerr << "problemid: " << problemid << std::endl;
        std::cerr << "options: " << options << std::endl;
        std::cerr << "answer:\n" << answer << std::endl;

        pcserver pcs;
        Problem pro(problem_set);
        pro.load(problemid);
        Answer ans(answer);

        if(pro.valid() && ans.valid())
            pcs.parse(pro.get(), ans.get());
        
        if(pcs.ok())
            if(options.find("quiet") != std::string::npos)
                res << pcs.get_output();
            else
                res << pcs.get_error() << pcs.get_output();
        else
            res << pro.get_error() << ans.get_error() << pcs.get_error();

        res.setHeader("Content-Type", "text/plain");
    }
    void dl_problem(Request &req, StreamResponse &res)
    {
        std::string const file = req.getUrl().substr(req.getUrl().find("/problem/") + 9);
        std::ifstream ifs("problem_set/" + problem_set + "/problem/" + file);
        if(ifs.fail()) {
            res << "404 Not found";
            res.setCode(404);
            res.setHeader("Content-Type", "text/plain");
        } else {
            res << ifs.rdbuf();
            res.setHeader("Content-Type", "image/x-portable-pixmap");
        }
    }
    void dl_position(Request &req, StreamResponse &res)
    {
        std::string const file = req.getUrl().substr(req.getUrl().find("/position/") + 10);
        std::ifstream ifs("problem_set/" + problem_set + "/position/" + file);
        if(ifs.fail()) {
            res << "404 Not found";
            res.setCode(404);
            res.setHeader("Content-Type", "text/plain");
        } else {
            res << ifs.rdbuf();
            res.setHeader("Content-Type", "text/plain");
        }
    }

    void setup()
    {
        addRoute("GET", "/SubmitForm", MyController, submit_form);
        addRoute("POST", "/SubmitAnswer", MyController, submit_answer);
//        addRoute("GET", "/problem/prob[0-9]{2}\\.ppm", MyController, dl_problem);
//        本当はこうしたい↑
        for(int i = 0; i < 100; i++) {
            std::ostringstream oss;
            oss << "/problem/prob" << std::setw(2) << std::setfill('0') << i << ".ppm";
            addRoute("GET", oss.str(), MyController, dl_problem);
        }
        for(int i = 0; i < 100; i++) {
            std::ostringstream oss;
            oss << "/position/prob" << std::setw(2) << std::setfill('0') << i << ".pos";
            addRoute("GET", oss.str(), MyController, dl_position);
        }
    }
};

volatile static bool running = true;
void signal_handler(int signum)
{
    if(running) {
        running = false;
    }
}

int main()
{
    const char* p = std::getenv("PORT");
    int const port = p ? std::stoi(p) : 8080;

    signal(SIGINT, signal_handler);
    MyController myController;
    Server server(port);
    server.registerController(&myController);
    server.setOption("enable_directory_listing", "false");
    server.start();

    while(running) {
        ;
    }
    server.stop();
    return 0;
}


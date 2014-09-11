#include <signal.h>
#include <mongoose/Server.h>
#include <mongoose/Controller.h>
#include <mongoose/Utils.h>

#include "server.hpp"
#include "tools.hpp"
#include "problem.hpp"
#include "answer.hpp"

using namespace Mongoose;

class MyController : public Controller
{
public:
    void submit_form(Request &req, StreamResponse &res)
    {
        std::ifstream ifs("res/SubmitForm.html");
        res << ifs.rdbuf();
        res.setHeader("Content-Type", "text/html");
    }
    void submit_answer(Request &req, StreamResponse &res)
    {
        std::string problem_set = "default";
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
    void setup()
    {
        addRoute("GET", "/SubmitForm", MyController, submit_form);
        addRoute("POST", "/SubmitAnswer", MyController, submit_answer);
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
    signal(SIGINT, signal_handler);
    MyController myController;
    Server server(8080);
    server.registerController(&myController);
    server.setOption("enable_directory_listing", "false");
    server.start();

    while(running) {
        ;
    }
    server.stop();
    return 0;
}


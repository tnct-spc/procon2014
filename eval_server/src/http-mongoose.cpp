#include <signal.h>
#include <mongoose/Server.h>
#include <mongoose/Controller.h>
#include <mongoose/Utils.h>

#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

#include "server.hpp"
#include "tools.hpp"
#include "problem.hpp"
#include "answer.hpp"
#include "config.hpp"

using namespace Mongoose;

class MyController : public Controller
{
    std::string problem_set = std::getenv("PCS_PROBSET") ? std::getenv("PCS_PROBSET") : "default";
    std::map<std::string, bool> user_active; // key: playerid
public:
    void show_usage(Request &req, StreamResponse &res)
    {
        std::ifstream ifs(PCS_RESDIR + "/usage.html");
        res << ifs.rdbuf();
        res.setHeader("Content-Type", "text/html");
    }
    void submit_form(Request &req, StreamResponse &res)
    {
        std::ifstream ifs(PCS_RESDIR + "/SubmitForm.html");
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

        if(user_active[playerid] && false) {
            res << "REJECTED !!\r\n";
        } else {
            user_active[playerid] = true;

            pcserver pcs;
            Problem pro(problem_set);
            pro.load(problemid);
            Answer ans(answer);
    
            if(pro.valid() && ans.valid())
                pcs.parse(pro.get(), ans.get());
            
            std::cerr << "Processed the answer. Now I feel a bit sleepy..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
    
            if(pcs.ok()) {
                if(options.find("verbose") != std::string::npos)
                    res << pcs.get_error() << pcs.get_output();
                else
                    res << pcs.get_output();
            } else
                res << "ERROR !!" << std::endl << pro.get_error() << ans.get_error() << pcs.get_error();
    
            user_active[playerid] = false;
        }
        res.setHeader("Content-Type", "text/plain");
    }
    void dl_problem(Request &req, StreamResponse &res)
    {
        std::cerr << req.getUrl() << std::endl;
        std::string const file = req.getUrl().substr(req.getUrl().find("/problem/") + 9);
        std::string const path = PCS_PROBDIR + "/" + problem_set + "/problem/" + file;
        std::ifstream ifs(path);
        if(ifs.fail()) {
            std::cerr << "File " + path + " could not be opened\n";
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
        std::cerr << req.getUrl() << std::endl;
        std::string const file = req.getUrl().substr(req.getUrl().find("/position/") + 10);
        std::string const path = PCS_PROBDIR + "/" + problem_set + "/position/" + file;
        std::ifstream ifs(path);
        if(ifs.fail()) {
            std::cerr << "File " + path + " could not be opened\n";
            res << "404 Not found";
            res.setCode(404);
            res.setHeader("Content-Type", "text/plain");
        } else {
            res << ifs.rdbuf();
            res.setHeader("Content-Type", "text/plain");
        }
    }
    void dl_answer(Request &req, StreamResponse &res)
    {
        std::cerr << req.getUrl() << std::endl;
        std::string const file = req.getUrl().substr(req.getUrl().find("/answer/") + 8);
        std::string const path = PCS_PROBDIR + "/" + problem_set + "/answer/" + file;
        std::ifstream ifs(path);
        if(ifs.fail()) {
            std::cerr << "File " + path + " could not be opened\n";
            res << "404 Not found";
            res.setCode(404);
            res.setHeader("Content-Type", "text/plain");
        } else {
            res << ifs.rdbuf();
            res.setHeader("Content-Type", "text/plain");
        }
    }
    void config(Request &req, StreamResponse &res)
    {
        std::cerr << req.getUrl() << std::endl;
        if(req.get("problem_set") != "") {
            problem_set = req.get("problem_set");
            res << "Problem set set to " + req.get("problem_set") << std::endl;
        }
        res.setHeader("Content-Type", "text/plain");
    }
    void debug(Request &req, StreamResponse &res)
    {
        auto ans = std::move(load_answer("/home/ntsc_j/procon2014/question/pp_prob01.ans"));
        for(auto a : ans) {
            for(auto b : a) {
                res << "(" << b.x << "," << b.y << ") ";
            }
            res << std::endl;
        }
        
    }

    void setup()
    {
        addRoute("GET", "/SubmitForm", MyController, submit_form);
        addRoute("POST", "/SubmitAnswer", MyController, submit_answer);
        addRoute("GET", "/", MyController, show_usage);
        addRoute("GET", "/index.html", MyController, show_usage);
        addRoute("GET", "/config", MyController, config);
        addRoute("GET", "/debug", MyController, debug);
//        addRoute("GET", "/problem/prob[0-9]{2}\\.ppm", MyController, dl_problem);
//        本当はこうしたい↑
        for(int i = 0; i < 100; i++) {
            std::ostringstream oss;
            oss << "/problem/prob" << std::setw(2) << std::setfill('0') << i << ".ppm";
            addRoute("GET", oss.str(), MyController, dl_problem);
            addRoute("POST", oss.str(), MyController, dl_problem);
        }
        for(int i = 0; i < 100; i++) {
            std::ostringstream oss;
            oss << "/position/prob" << std::setw(2) << std::setfill('0') << i << ".pos";
            addRoute("GET", oss.str(), MyController, dl_position);
            addRoute("POST", oss.str(), MyController, dl_position);
        }
        for(int i = 0; i < 100; i++) {
            std::ostringstream oss;
            oss << "/answer/prob" << std::setw(2) << std::setfill('0') << i << ".ans";
            addRoute("GET", oss.str(), MyController, dl_answer);
            addRoute("POST", oss.str(), MyController, dl_answer);
        }
    }
};

volatile static bool running = true;
void signal_handler(int signum)
{
    if(running) {
        std::cerr << "Exiting...\n";
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    server.stop();
    return 0;
}


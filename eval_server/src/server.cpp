#include <iomanip>
#include "server.hpp"
#include "tools.hpp"
#include "test_tool.hpp"

// 🍣🍣🍣  パーサ  🍣🍣🍣
void pcserver::parse(question_data const& problem, answer_type const& answer)
{
    // outputに本番サーバと同じ情報、outerrにそれ以外を吐く
    std::cerr << "TARGET CAPTURED...\n";
    test_tool::emulator emu(problem);
    std::cerr << "BODY SENSOR\n";
    auto const result = emu.start(answer);
    std::cerr << "EMULATED EMULATED EMULATED\n";

    output << "ACCEPTED " << std::setw(2) << std::setfill('0') << result.wrong << std::endl;
    sane = true;
}


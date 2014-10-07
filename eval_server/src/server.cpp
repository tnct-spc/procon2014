#include <iomanip>
#include "server.hpp"
#include "tools.hpp"
#include "test_tool.hpp"

// 🍣🍣🍣  パーサ  🍣🍣🍣
void pcserver::parse(question_data const& problem, answer_type const& answer)
{
    // outputに本番サーバと同じ情報、outerrにそれ以外を吐く
    test_tool::emulator emu(problem);
    auto const result = emu.start(answer);

    output << "ACCEPTED " << std::setw(2) << std::setfill('0') << result.wrong;
    sane = true;
}


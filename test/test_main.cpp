
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <map>
#include <string>

#include "CmdArg.h"
#include "SimulateProcess.hpp"
#include "SimulateMemory.hpp"

void summary(const SimulateMemory& memory, size_t num_ops)
{
    std::cout << "\n### Final Page Table\n"
              << std::endl;
    memory.dumpPageTable();
    std::cout << "## Summary" << std::endl;
    std::cout << "- Number of Page Faults on Read: " << memory.getNumPageFaultRead() << std::endl;
    std::cout << "- Number of Page Faults on Write: " << memory.getNumPageFaultWrite() << std::endl;
    std::cout << "- Number of Page Faults on Exec: " << memory.getNumPageFaultExec() << std::endl;
    std::cout << "- Number of Page Faults: " << memory.getNumPageFault() << std::endl;
    std::cout << "- Page Fault Rate: " << (double)memory.getNumPageFault() / num_ops << std::endl
              << std::endl;
}

void suit(const SimulateMemory& memory, AlgoBase* algo, const AccessSeq_t& acc)
{
    std::cout << "## Test Details\n"
              << std::endl;
    for (auto i = 0; i < acc.size(); i++) {
        std::cout << "### Step " << i << "\n\nCurrent Page Table:" << std::endl;
        memory.dumpPageTable();
        algo->access(acc[i].first, acc[i].second);
        std::cout << "\n---" << std::endl;
    }
    summary(memory, acc.size());
}

void suit_opt()
{
    AccessSeq_t acc = {
        { 7, PF_READ }, { 0, PF_READ }, { 1, PF_READ }, { 2, PF_READ }, { 0, PF_READ },
        { 3, PF_READ }, { 0, PF_READ }, { 4, PF_RW }, { 2, PF_READ }, { 3, PF_READ },
        { 0, PF_READ }, { 3, PF_READ }, { 2, PF_READ }, { 1, PF_READ }, { 2, PF_READ },
        { 0, PF_READ }, { 1, PF_READ }, { 7, PF_READ }, { 0, PF_READ }, { 1, PF_READ }
    };
    SimulateMemory memory(3);
    AlgoOPT opt(&memory, 20, acc);
    suit(memory, &opt, acc);
}

void suit_fifo(size_t num_ppages)
{

    AccessSeq_t acc = {
        { 3, PF_READ }, { 2, PF_READ }, { 1, PF_READ }, { 0, PF_READ },
        { 3, PF_READ }, { 2, PF_READ }, { 4, PF_READ }, { 3, PF_READ },
        { 2, PF_READ }, { 1, PF_READ }, { 0, PF_READ }, { 4, PF_READ }
    };
    SimulateMemory memory(num_ppages);
    AlgoFIFO fifo(&memory);
    suit(memory, &fifo, acc);
}

void suit_lru()
{
    AccessSeq_t acc = {
        { 1, PF_READ }, { 8, PF_READ }, { 1, PF_READ }, { 7, PF_READ }, { 8, PF_READ },
        { 2, PF_READ }, { 7, PF_READ }, { 2, PF_READ }, { 1, PF_READ }, { 8, PF_READ },
        { 3, PF_READ }, { 8, PF_READ }, { 2, PF_READ }, { 1, PF_READ }, { 3, PF_READ },
        { 1, PF_READ }, { 7, PF_READ }, { 1, PF_READ }, { 3, PF_READ }, { 7, PF_READ }
    };
    SimulateMemory memory(4);
    AlgoLRU lru(&memory);
    suit(memory, &lru, acc);
}

void suit_clock()
{
    SimulateMemory memory(3);
    AlgoClock clock(&memory);

    AccessSeq_t acc = {
        { 7, PF_READ }, { 0, PF_READ }, { 1, PF_READ }, { 2, PF_READ }, { 0, PF_READ },
        { 3, PF_READ }, { 0, PF_READ }, { 4, PF_RW }, { 2, PF_READ }, { 3, PF_READ },
        { 0, PF_READ }, { 3, PF_READ }, { 2, PF_READ }, { 1, PF_READ }, { 2, PF_READ },
        { 0, PF_READ }, { 1, PF_READ }, { 7, PF_READ }, { 0, PF_READ }, { 1, PF_READ }
    };
    suit(memory, &clock, acc);
}

void suit_optclock()
{
    SimulateMemory memory(3);
    AlgoOptClock clock(&memory);

    AccessSeq_t acc = {
        { 7, PF_READ }, { 0, PF_READ }, { 1, PF_READ }, { 2, PF_READ }, { 0, PF_READ },
        { 3, PF_READ }, { 0, PF_READ }, { 4, PF_RW }, { 2, PF_READ }, { 3, PF_READ },
        { 0, PF_READ }, { 3, PF_READ }, { 2, PF_READ }, { 1, PF_READ }, { 2, PF_READ },
        { 0, PF_READ }, { 1, PF_READ }, { 7, PF_READ }, { 0, PF_READ }, { 1, PF_READ }
    };
    suit(memory, &clock, acc);
}

auto modeStr = [](int mode) {
    switch (mode) {
    case MODE_OPT:
        return "OPT";
    case MODE_FIFO:
        return "FIFO";
    case MODE_LRU:
        return "LRU";
    case MODE_CLOCK:
        return "Clock";
    case MODE_OPTCLOCK:
        return "OptClock";
    case MODE_ALL:
        return "All";
    default:
        return "Unknown";
    }
};

auto suit(ProgramMode mode, size_t psize, size_t vsize, const AccessSeq_t& acc)
{
    SimulateMemory memory(psize);
    AlgoBase* algo = nullptr;
    switch (mode) {
    case MODE_OPT:
        algo = new AlgoOPT(&memory, vsize, acc);
        break;
    case MODE_FIFO:
        algo = new AlgoFIFO(&memory);
        break;
    case MODE_LRU:
        algo = new AlgoLRU(&memory);
        break;
    case MODE_CLOCK:
        algo = new AlgoClock(&memory);
        break;
    case MODE_OPTCLOCK:
        algo = new AlgoOptClock(&memory);
        break;
    default:
        std::cerr << "Unknown mode: " << mode << std::endl;
        exit(-3);
    }
    std::cout << "# " << modeStr(mode) << "\n"
              << std::endl;
    suit(memory, algo, acc);
    delete algo;
    return std::tuple<size_t, size_t, size_t, size_t> { memory.getNumPageFault(), memory.getNumPageFaultRead(), memory.getNumPageFaultWrite(), memory.getNumPageFaultExec() };
}

int main(int argc, char* argv[])
{
    CmdArgParser cmdarg(argc, argv);
    if (!cmdarg.getOutputFile().empty()) {
        freopen(cmdarg.getOutputFile().c_str(), "w", stdout);
    }
    if (!cmdarg.getInputFile().empty()) {
        freopen(cmdarg.getInputFile().c_str(), "r", stdin);
    }

    if (cmdarg.getMode() == MODE_SELFTEST) {
        std::cout << "---\n"
                     "title: PgSub Test (Self Test Mode)\n"
                     "---\n"
                  << std::endl;
        std::cout << "# Test OPT\n"
                  << std::endl;
        suit_opt();
        std::cout << "# Test FIFO (P=3)\n"
                  << std::endl;
        suit_fifo(3);
        std::cout << "# Test FIFO (P=4)\n"
                  << std::endl;
        suit_fifo(4);
        std::cout << "# Test LRU\n"
                  << std::endl;
        suit_lru();
        std::cout << "# Test Clock\n"
                  << std::endl;
        suit_clock();
        std::cout << "# Test OptClock\n"
                  << std::endl;
        suit_optclock();
        return 0;
    }

    AccessSeq_t acc;
    if (cmdarg.getNumOps()) {
        acc = SimulateProcess(cmdarg.getVSize()).random(cmdarg.getNumOps())();
    } else {
        pgidx_t vpn, access_type;
        while (std::cin >> vpn >> access_type) {
            if (vpn > cmdarg.getVSize()) {
                std::cerr << "Invalid VPN: " << vpn << std::endl;
                exit(-2);
            }
            if (access_type > 7) {
                std::cerr << "Invalid Access Type: " << access_type << std::endl;
                exit(-2);
            }
            acc.push_back({ vpn, access_type });
        }
    }

    // clang-format off
    std::cout << "---\n"
                 "title: PgSub Test\n" <<
                 "mode: " << modeStr(cmdarg.getMode()) << "\n"
                "vsize: " << cmdarg.getVSize() << "\n"
                "psize: " << cmdarg.getPSize() << "\n"
                "numops: " << acc.size() << "\n"
                 "---\n"
              << std::endl;
    // clang-format on
    if (cmdarg.getMode() == MODE_ALL) {
        auto opt = suit(MODE_OPT, cmdarg.getPSize(), cmdarg.getVSize(), acc);
        auto fifo = suit(MODE_FIFO, cmdarg.getPSize(), cmdarg.getVSize(), acc);
        auto lru = suit(MODE_LRU, cmdarg.getPSize(), cmdarg.getVSize(), acc);
        auto clock = suit(MODE_CLOCK, cmdarg.getPSize(), cmdarg.getVSize(), acc);
        auto optclock = suit(MODE_OPTCLOCK, cmdarg.getPSize(), cmdarg.getVSize(), acc);
        std::cout << "# Total Summary\n"
                  << std::endl;
        std::cout << "|Mode|PF|PF Read|PF Write|PF Exec|PF Rate|\n"
                     "|---|---|---|---|---|---|\n"
                  << "|OPT|" << std::get<0>(opt) << "|" << std::get<1>(opt) << "|" << std::get<2>(opt) << "|" << std::get<3>(opt) << "|" << (double)std::get<0>(opt) / acc.size() << "|\n"
                  << "|FIFO|" << std::get<0>(fifo) << "|" << std::get<1>(fifo) << "|" << std::get<2>(fifo) << "|" << std::get<3>(fifo) << "|" << (double)std::get<0>(fifo) / acc.size() << "|\n"
                  << "|LRU|" << std::get<0>(lru) << "|" << std::get<1>(lru) << "|" << std::get<2>(lru) << "|" << std::get<3>(lru) << "|" << (double)std::get<0>(lru) / acc.size() << "|\n"
                  << "|Clock|" << std::get<0>(clock) << "|" << std::get<1>(clock) << "|" << std::get<2>(clock) << "|" << std::get<3>(clock) << "|" << (double)std::get<0>(clock) / acc.size() << "|\n"
                  << "|OptClock|" << std::get<0>(optclock) << "|" << std::get<1>(optclock) << "|" << std::get<2>(optclock) << "|" << std::get<3>(optclock) << "|" << (double)std::get<0>(optclock) / acc.size() << "|\n"
                  << std::endl;
    } else {
        suit(cmdarg.getMode(), cmdarg.getPSize(), cmdarg.getVSize(), acc);
    }
    return 0;
}
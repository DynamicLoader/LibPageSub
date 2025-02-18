#ifndef SIMULATE_PROCESS_HPP
#define SIMULATE_PROCESS_HPP

#include <libpgsub.h>
#include <vector>
#include <random>

using namespace LibPGSub;

class SimulateProcess {
private:
    std::vector<std::pair<pgidx_t, pf_t>> _pgaccess_sequence;
    pgidx_t _num_vpages;
    

public:

    SimulateProcess(pgidx_t num_vpages)
    : _num_vpages(num_vpages)
    {
    }
    ~SimulateProcess() = default;

    SimulateProcess& random(size_t _num_ops){
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<pgidx_t> dis_pg(0, _num_vpages - 1);
        std::uniform_int_distribution<short> dis_pf(0, 4);


        for (size_t i = 0; i < _num_ops; ++i) {
            _pgaccess_sequence.push_back(std::make_pair(dis_pg(gen), short2pf(dis_pf(gen))));
        }
        return *this;
    }

    const auto& operator()() const
    {
        return _pgaccess_sequence;
    }

private:
    pf_t short2pf(short s) {
        switch(s){
            case 0:
                return PF_READ;
            case 1:
                return PF_WRITE;
            case 2:
                return PF_EXEC;
            case 3:
                return PF_RW;
            case 4:
                return PF_RX;
        }
        return 0;
    }
};

#endif // SIMULATE_PROCESS_HPP
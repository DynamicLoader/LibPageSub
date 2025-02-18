/**
 * @file OPT.hpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-10-15
 *
 * @copyright Copyright (c) 2024
 *
 * @details This file contains the OPT algorithm implementation.
    Note that OPT is not a practical algorithm, but it is useful for testing purposes.
 */

#ifndef __LIBPGSUB_ALGO_OPT_HPP__
#define __LIBPGSUB_ALGO_OPT_HPP__

#include "../types.h"
#include "Base.h"
#include "../Exceptions.h"

#include <string>
#include <map>

PGSUB_NAMESPACE_BEGIN

class AlgoOPT : public AlgoBase {
private:
    pgidx_t _num_vpages;
    std::vector<size_t> _next_access;
    AccessSeq_t _access_sequence;
    size_t _access_index = 0;

    std::map<pgidx_t, pgidx_t>
        _reverse_page_table; // PPN -> VPN (This is not to be used in real
                             // hardware), and in test env, one PPN is mapped to
                             // only one VPN

public:
    AlgoOPT(AbstractMemory* memory, const pgidx_t& num_vpages, const AccessSeq_t& acc)
        : AlgoBase(memory)
        , _num_vpages(num_vpages)
        , _access_sequence(acc)
    {
        _next_access.resize(num_vpages, -1);
    }

    ~AlgoOPT() = default;

    void access(const pgidx_t& vpage, pf_t access_type) override
    {
        _process(vpage, access_type);
        try {
            _memory->access(vpage, access_type);
        } catch (PageFaultNotLoaded& e) {
            auto vit = _findVictim();
            _memory->load(vpage, vit.first, vit.second);
            _reverse_page_table[vit.first] = vpage;
            _memory->access(vpage, access_type); // check again
        }
        // There could be other exceptions such as access violation which is
        // throw by AbstractMemory.access(), delegate them to the caller
    }

private:
    // Get the virtual page number of a physical page
    // Note that this is not to be used in real hardware
    pgidx_t _getVPage(const pgidx_t& ppage)
    {
        auto ret = _reverse_page_table.find(ppage);
        if (ret == _reverse_page_table.end()) {
            return INVALID_PAGE;
        }
        return ret->second;
    }

    // Find a physical page to be replaced
    std::pair<pgidx_t, pgidx_t> _findVictim()
    {
        pgidx_t vit = _memory->getFreePPage();
        if (vit != INVALID_PAGE) {
            return { vit, INVALID_PAGE };
        }
        size_t latest = 0;
        pgidx_t evict_vpn = INVALID_PAGE;
        for (auto i = 0; i < _memory->getNumPPages(); ++i) {
            auto j = _getVPage(i);
            if (_next_access[j] != -1) {
                if (vit == INVALID_PAGE) {
                    vit = i;
                    latest = _next_access[j];
                    evict_vpn = j;
                } else {
                    if (_next_access[j] > latest) {
                        vit = i;
                        latest = _next_access[j];
                        evict_vpn = j;
                    }
                }
            } else {
                vit = i;
                evict_vpn = j;
                break;
            }
        }
        return { vit, evict_vpn };
    }

    void _process(const pgidx_t& vpage, pf_t access_type)
    {
        if(vpage >= _num_vpages) {
            throw SimulateFaultInvalidVPN(std::to_string(vpage));
        }
        if (_access_index >= _access_sequence.size()) {
            throw SimulateFaultStepOutOfBound("# " + std::to_string(_access_index));
        }
        auto x = _access_sequence[_access_index];
        if (x.first != vpage || x.second != access_type) {
            throw SimulateFaultStepNotSync(std::to_string(_access_index));
        }
        _access_index++;
        _next_access.assign(_num_vpages, -1);
        for (auto i = _access_index; i < _access_sequence.size(); ++i) {
            if (i < _next_access[_access_sequence[i].first]) {
                _next_access[_access_sequence[i].first] = i;
            }
        }
    }
};

PGSUB_NAMESPACE_END

#endif
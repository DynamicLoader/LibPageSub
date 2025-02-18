/**
 * @file LRU.hpp
 * @author your name (you@domain.com)
 * @brief A Least Recently Used (LRU) algorithm implementation.
 * @version 0.1
 * @date 2024-10-16
 * @details The LRU in hardware usually implemented by a counter and a auto-loading mechanism.
    * The counter is updated on every memory access. The counter is used to determine the least recently used page.
    * Here we use a map to simulate the counter and hook the access function to update the counter.
    @note The implementation is not to be used in real OS or hardware.
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include "../types.h"
#include "../Exceptions.h"
#include "Base.h"

#include <algorithm>
#include <map>

PGSUB_NAMESPACE_BEGIN

class AlgoLRU : public AlgoBase {
private:
    std::map<pgidx_t, size_t> _vpc; // VPN -> counter
    size_t _counter = 0;

public:
    using AlgoBase::AlgoBase;

    ~AlgoLRU() = default;

    void access(const pgidx_t& vpn, pf_t access_type) override
    {
        _vpc[vpn] = _counter++;
        try {
            _memory->access(vpn, access_type);
        } catch (PageFaultNotLoaded& e) {
            auto ppage = _memory->getFreePPage();
            pgidx_t lru = INVALID_PAGE;
            if (ppage == INVALID_PAGE) {
                lru = _getLRU();
                if (lru == INVALID_PAGE) {
                    throw std::runtime_error("[x] No page to evict");
                }
                ppage = _memory->getPPage(lru);
            }
            _vpc.erase(lru);
            _memory->load(vpn, ppage, lru);
            _memory->access(vpn, access_type);
        }
    }

private:
    pgidx_t _getLRU() const
    {
        auto ret = std::min_element(_vpc.begin(), _vpc.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            });
        if (ret == _vpc.end()) {
            return INVALID_PAGE;
        }
        return ret->first;
    }
};

PGSUB_NAMESPACE_END
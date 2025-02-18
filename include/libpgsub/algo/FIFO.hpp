/**
 * @file FIFO.hpp
 * @author your name (you@domain.com)
 * @brief This file contains the FIFO algorithm implementation.
 * @version 0.1
 * @date 2024-10-16
 *
 * @copyright Copyright (c) 2024
 * @details
 *
 */

#pragma once

#include "../types.h"
#include "../Exceptions.h"

#include "Base.h"

#include <deque>

PGSUB_NAMESPACE_BEGIN

class AlgoFIFO : public AlgoBase {
private:
    std::deque<pgidx_t> _pg_fifo;

public:
    using AlgoBase::AlgoBase;

    ~AlgoFIFO() = default;

    void access(const pgidx_t& vpage, pf_t access_type) override
    {
        try {
            _memory->access(vpage, access_type);
        } catch (PageFaultNotLoaded& e) {
            auto vit = _findVictim();
            _memory->load(vpage, vit.first, vit.second);
            // we had remapped the page, correct the FIFO
            for (auto i = _pg_fifo.begin(); i < _pg_fifo.end(); i++) {
                if (*i == vit.second) {
                    _pg_fifo.erase(i);
                    break;
                }
            }
            _pg_fifo.push_back(vpage);
            _memory->access(vpage, access_type); // access again
        }
    }

private:
    std::pair<pgidx_t, pgidx_t> _findVictim()
    {
        pgidx_t vit = _memory->getFreePPage();
        if (vit != INVALID_PAGE) {
            return { vit, INVALID_PAGE };
        }
        auto v = _pg_fifo.front();
        _pg_fifo.pop_front();
        return { _memory->getPPage(v), v };
    }
};

PGSUB_NAMESPACE_END
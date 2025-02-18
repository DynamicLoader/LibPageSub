/**
 * @file Clock.hpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-10-16
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include "../types.h"
#include "../Exceptions.h"
#include "Base.h"

#include <forward_list>

PGSUB_NAMESPACE_BEGIN

class AlgoClock : public AlgoBase {
protected:
    template <typename T>
    class RingList {
    private:
        std::forward_list<T> _list;
        typename std::forward_list<T>::iterator _hand;

    public:
        RingList()
            : _hand(_list.before_begin())
        {
        }

        void insert(const T& value)
        {
            if(_list.empty()){
                _hand = _list.insert_after(_hand, value);
            }else{
                _list.insert_after(_hand, *_hand);
                *_hand = value;
                next();
            }
        }

        typename std::forward_list<T>::iterator begin()
        {
            return _list.begin();
        }

        typename std::forward_list<T>::iterator end()
        {
            return _list.end();
        }

        typename std::forward_list<T>::iterator next()
        {
            if (_hand == _list.before_begin()) {
                _hand = _list.begin();
            } else {
                ++_hand;
                if (_hand == _list.end()) {
                    _hand = _list.begin();
                }
            }
            return _hand;
        }

        typename std::forward_list<T>::iterator current()
        {
            return _hand;
        }

        bool empty() const
        {
            return _list.empty();
        }

        void reset()
        {
            _hand = _list.before_begin();
        }
    };

    RingList<pgidx_t> _alloc_pages;

public:
    using AlgoBase::AlgoBase;

    ~AlgoClock() = default;

    void access(const pgidx_t& vpn, pf_t access_type) override
    {
        try {
            _memory->access(vpn, access_type);
        } catch (PageFaultNotLoaded& e) {
            auto v = _process(vpn, access_type);
            _memory->load(vpn, v.first, v.second);
            _memory->access(vpn, access_type);
        }
    }

protected:
    virtual std::pair<pgidx_t, pgidx_t> _process(const pgidx_t& vpn, pf_t access_type)
    {
        pgidx_t ppn = _memory->getFreePPage();
        if (ppn != INVALID_PAGE) {
            _alloc_pages.insert(vpn);
            return { ppn, INVALID_PAGE };
        }
        auto c = _alloc_pages.current(), n = c;
        for (int i = 0; i < 2; ++i) {
            do {
                auto pf = _memory->getVFlag(*n);
                if ((pf & PF_ACCESSED) == 0) {
                    std::pair<pgidx_t, pgidx_t> ret = { _memory->getPPage(*n), *n };
                    *n = vpn;
                    _alloc_pages.next();
                    return ret;
                }
                _memory->setVFlag(*n, pf & ~PF_ACCESSED);
                n = _alloc_pages.next();
            } while (n != c);
        }
        return { INVALID_PAGE, INVALID_PAGE }; // Make compiler happy
    }
};

class AlgoOptClock : public AlgoClock {
public:
    using AlgoClock::AlgoClock;

protected:
    virtual std::pair<pgidx_t, pgidx_t> _process(const pgidx_t& vpn, pf_t access_type) override
    {
        pgidx_t ppn = _memory->getFreePPage();
        if (ppn != INVALID_PAGE) {
            _alloc_pages.insert(vpn);
            return { ppn, INVALID_PAGE };
        }
        auto c = _alloc_pages.current(), n = c;
        for (int i = 0; i < 4; ++i) {
            pf_t flag = (i & 0x1 ? PF_DIRTY : 0);
            do {
                auto pf = _memory->getVFlag(*n);
                if ((pf & (PF_ACCESSED | PF_DIRTY)) == flag) {
                    std::pair<pgidx_t, pgidx_t> ret = { _memory->getPPage(*n), *n };
                    *n = vpn;
                    _alloc_pages.next();
                    return ret;
                }
                if (i == 1) {
                    _memory->setVFlag(*n, pf & ~PF_ACCESSED);
                }
                n = _alloc_pages.next();
            } while (n != c);
        }
        return { INVALID_PAGE, INVALID_PAGE }; // Make compiler happy
    }
};

PGSUB_NAMESPACE_END

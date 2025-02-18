#ifndef SIMULATE_MEMORY_HPP
#define SIMULATE_MEMORY_HPP

#include <cstddef>
#include <iostream>
#include <map>
#include <string>

#include <libpgsub.h>

using namespace LibPGSub;

class SimulateMemory : public LibPGSub::AbstractMemory {
private:
    size_t _num_ppages;

    size_t _pgfault_read_count = 0;
    size_t _pgfault_write_count = 0;
    size_t _pgfault_exec_count = 0;

    std::map<pgidx_t, std::pair<pf_t, pgidx_t>> _page_table; // VPN -> PPN
    std::vector<bool> _palloc_table; // PPN -> isAllocated

    std::string access_type_to_string(pf_t access_type) const
    {
        std::string str = "---";
        if (access_type & PF_READ) {
            str[0] = 'R';
        }
        if (access_type & PF_WRITE) {
            str[1] = 'W';
        }
        if (access_type & PF_EXEC) {
            str[2] = 'X';
        }
        return str;
    }

    std::string pf_to_string(pf_t pf) const
    {
        std::string str = "---";
        if (pf & PF_ACCESSED) {
            str[0] = 'A';
        }
        if (pf & PF_DIRTY) {
            str[1] = 'D';
        }
        if (pf & PF_VALID) {
            str[2] = 'V';
        }
        return str;
    }

public:
    SimulateMemory(size_t num_ppages)
        : _num_ppages(num_ppages)
    {
        _palloc_table.resize(num_ppages, false);
    }
    ~SimulateMemory() = default;

    void access(const LibPGSub::pgidx_t& vpn,
        LibPGSub::pf_t access_type) override
    {
        std::cout << "Accessing VPN # " << vpn << " with "
                  << access_type_to_string(access_type) << std::endl;
        auto ret = _page_table.find(vpn);
        if (ret == _page_table.end() || (ret->second.first & PF_VALID) == 0) {
            if (access_type & PF_WRITE) {
                std::cout << "**Page Fault on Write**" << std::endl;
                _pgfault_write_count++;
                throw LibPGSub::PageFaultWriteNotLoaded(std::to_string(vpn));
            } else if (access_type & PF_READ) {
                std::cout << "**Page Fault on Read**" << std::endl;
                _pgfault_read_count++;
                throw LibPGSub::PageFaultReadNotLoaded(std::to_string(vpn));
            } else {
                std::cout << "**Page Fault on Exec**" << std::endl;
                _pgfault_exec_count++;
                throw LibPGSub::PageFaultExecNotLoaded(std::to_string(vpn));
            }
        }

        if (access_type & PF_WRITE) {
            _page_table[vpn].first |= PF_DIRTY;
        } else {
            _page_table[vpn].first |= PF_ACCESSED;
        }
        std::cout << "_Now the VPN has PPN # " << _page_table[vpn].second
                  << " with flags " << pf_to_string(_page_table[vpn].first)
                  << "_" << std::endl;
    }

    void load(const LibPGSub::pgidx_t& vpn,
        const LibPGSub::pgidx_t& ppage, const pgidx_t& evict_vpn) override
    {
        std::cout << "Loading VPN # " << vpn << " with PPN # " << ppage
                  << std::endl;
        if (ppage > _num_ppages) {
            throw LibPGSub::SimulateFaultInvalidPPN(std::to_string(ppage));
        }
        if (evict_vpn != INVALID_PAGE && _page_table.find(evict_vpn) != _page_table.end()) {
            pf_t old_pf = _page_table[evict_vpn].first;
            std::cout << "_PPN # " << _page_table[evict_vpn].second << " already loaded to VPN # " << evict_vpn
                      << " and flags = " << pf_to_string(old_pf) << ", Evicting!_" << std::endl;
            if (old_pf & PF_DIRTY) {
                std::cout << "_Writing back dirty page to disk_" << std::endl;
            } else {
                std::cout << "_No need to write back_" << std::endl;
            }
            // _page_table.erase(evict_vpn);
            _page_table[evict_vpn].first &= ~PF_VALID & ~PF_DIRTY & ~PF_ACCESSED;
        }
        _page_table[vpn] = { PF_VALID, ppage };
        _palloc_table[ppage] = true;
    }

    size_t getNumPPages() const override { return _num_ppages; }

    pgidx_t getFreePPage() override
    {
        auto ret = std::find(_palloc_table.begin(), _palloc_table.end(), false);
        if (ret != _palloc_table.end()) {
            return std::distance(_palloc_table.begin(), ret);
        }
        return INVALID_PAGE;
    }

    pf_t getVFlag(const pgidx_t& vpn) const override
    {
        auto ret = _page_table.find(vpn);
        if (ret == _page_table.end()) {
            return 0;
        }
        return ret->second.first;
    }

    pf_t setVFlag(const pgidx_t& vpn, const pf_t& flag) override
    {
        auto ret = _page_table.find(vpn);
        if (ret == _page_table.end()) {
            return 0; // No previous flag
        }
        auto old_flag = ret->second.first;
        ret->second.first = flag;
        return old_flag;
    }

    pgidx_t getPPage(const pgidx_t& vpn) override
    {
        auto ret = _page_table.find(vpn);
        if (ret == _page_table.end()) {
            return INVALID_PAGE;
        }
        return ret->second.second;
    }

    // For testing purposes

    size_t getNumPageFaultRead() const { return _pgfault_read_count; }
    size_t getNumPageFaultWrite() const { return _pgfault_write_count; }
    size_t getNumPageFaultExec() const { return _pgfault_exec_count; }
    size_t getNumPageFault() const
    {
        return _pgfault_read_count + _pgfault_write_count + _pgfault_exec_count;
    }

    void dumpPageTable() const
    {
        std::string str;
        str += "|VPN\t|PPN\t|Flags\t|\n";
        str += "|-|-|-|\n";
        for (auto& [vpn, pte] : _page_table) {
            str += "|" + std::to_string(vpn) + "\t|" + std::to_string(pte.second) + "\t|" + pf_to_string(pte.first) + "\t|\n";
        }
        std::cout << str << std::endl;
    }

    const auto& getPageTable() const { return _page_table; }
};

#endif // SIMULATE_MEMORY_HPP
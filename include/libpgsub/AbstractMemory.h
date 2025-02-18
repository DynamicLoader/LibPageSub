#pragma once

#include "macro.h"
#include "types.h"

PGSUB_NAMESPACE_BEGIN

// enum VPageType{
//     VPageType_4K,
//     VPageType_2M,
//     VPageType_1G
// };

// struct VPageInfo{
//     pgidx_t ppage;
//     pf_t flag;
//     VPageType type;

//     VPageInfo(pgidx_t ppage, pf_t flag, VPageType type)
//         : ppage(ppage), flag(flag), type(type) {}
// };

/**
 * @brief An Abstract class for memory management.
 * @details The class should handle the MMU, which is responsible for loading, evicting, and accessing pages.
 */
class AbstractMemory {
public:
    AbstractMemory() = default;
    virtual ~AbstractMemory() = default;

    /**
     * @brief Access a page.
     * @details In real hardware, the function is used to check the page table and raise a further detail exception on the permission and so on
     * @param vpn Virtual memory page to be accessed.
     * @param access_type Type of access. It can be a combination of PF_READ, PF_WRITE, PF_EXEC.
     */
    virtual void access(const pgidx_t& vpn, pf_t access_type) = 0;

    /**
     * @brief Load a page of virtual memory into physical memory.
     * @details When a page with Dirty flag is loaded, the page should be written back to the disk by the implementation.
     * @param vpn Virtual memory page to be loaded.
     * @param ppn Physical memory page to load.
     */
    virtual void load(const pgidx_t& vpn, const pgidx_t& ppn, const pgidx_t& evict_vpn = INVALID_PAGE) = 0;

    /**
     * @brief Get the physical page of a virtual page.
     *
     * @param vpn Virtual memory page.
     * @return pgidx_t Physical memory page.
     */
    virtual pgidx_t getPPage(const pgidx_t& vpn) = 0;

    /**
     * @brief Get flag of a virtual page.
     *
     * @param vpn Virtual memory page.
     * @return pf_t Flag of the page. Negative value means not loaded.
     */
    virtual pf_t getVFlag(const pgidx_t& vpn) const = 0;


    /**
     * @brief Set the flag of a virtual page.
     *
     * @param vpn Virtual memory page.
     * @param flag Flag to be set. (Overwrite)
     * @return pf_t Previous flag.
     */
    virtual pf_t setVFlag(const pgidx_t& vpn, const pf_t& flag) = 0;

    /**
     * @brief Get the Free physical page
     *
     * @return pgidx_t Physical page index
     */
    virtual pgidx_t getFreePPage() = 0;

    /**
     * @brief Get the number of physical pages.
     *
     * @return size_t Number of physical pages.
     */
    virtual size_t getNumPPages() const = 0;

    /**
     * @brief Reset the memory.
     *
     */
    virtual void reset() { }
};

PGSUB_NAMESPACE_END
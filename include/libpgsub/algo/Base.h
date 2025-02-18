#pragma once

#include "../macro.h"
#include "../AbstractMemory.h"

PGSUB_NAMESPACE_BEGIN

class AlgoBase {

protected:
    AbstractMemory* _memory;

public:
    /**
     * @brief Construct a new Page Sub Algo Base object
     * 
     * @param memory Pointer to Impl of AbstractMemory object
     * @param num_pages Number of pages of PHYSICAL memory
     */
    AlgoBase(AbstractMemory* memory)
        : _memory(memory)
    {
    }

    virtual ~AlgoBase() = default;

    /**
     * @brief Mark a page as accessed with a specific access type.
     * @details In real haredware, the function is invoked from ESR of PageFault.
     * @param vpage Virtual page to be accessed.
     * @param access_type Access type. It can be a combination of PF_ACCESSED, PF_DIRTY
     */
    virtual void access(const pgidx_t& vpage, pf_t access_type) = 0;
};

PGSUB_NAMESPACE_END
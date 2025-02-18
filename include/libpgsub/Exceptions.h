#pragma once

#include "macro.h"
#include <stdexcept>
#include <string>

PGSUB_NAMESPACE_BEGIN

class PageFaultNotLoaded : public std::runtime_error {
public:
    PageFaultNotLoaded(const std::string& msg)
        : std::runtime_error(msg)
    {
    }
};

#define PGSUB_EXCEPTION_HELPER(name, base, msg) \
    class name : public base {                  \
    public:                                     \
        name(const std::string& detail)         \
            : base(msg + detail)                \
        {                                       \
        }                                       \
    }

PGSUB_EXCEPTION_HELPER(PageFaultReadNotLoaded, PageFaultNotLoaded, "Page Fault - Read Not Loaded: ");
PGSUB_EXCEPTION_HELPER(PageFaultWriteNotLoaded, PageFaultNotLoaded, "Page Fault - Write Not Loaded");
PGSUB_EXCEPTION_HELPER(PageFaultExecNotLoaded, PageFaultNotLoaded, "Page Fault - Exec Not Loaded: ");

PGSUB_EXCEPTION_HELPER(SimulateFaultStepNotSync, std::runtime_error, "Simulate Fault - Step Not Sync: ");
PGSUB_EXCEPTION_HELPER(SimulateFaultStepOutOfBound, std::runtime_error, "Simulate Fault - Step Out Of Bound: ");
PGSUB_EXCEPTION_HELPER(SimulateFaultInvalidVPN, std::runtime_error, "Simulate Fault - Invalid VPN: ");
PGSUB_EXCEPTION_HELPER(SimulateFaultInvalidPPN, std::runtime_error, "Simulate Fault - Invalid PPN: ");

PGSUB_NAMESPACE_END
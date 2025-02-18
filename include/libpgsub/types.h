#pragma once

#include <cstdint>
#include <vector>
#include <utility>

#include "macro.h"

#ifndef CONFIG_WITH_CUSTOM_TYPES_HEADER

PGSUB_NAMESPACE_BEGIN

/* 
    On most systems, the virtual address space is not over 48bits (on RV64 it's 39bits when using SV39), and one page is usually 4KB (12bits).
    To simplify the implementation, we use 32bits for the page index. Users can change this to 64bits if needed in the custom types header.
*/
using pgidx_t = uint32_t;


// Page flags
using pf_t = uint8_t;

constexpr pf_t PF_READ = 0x01;
constexpr pf_t PF_WRITE = 0x02;
constexpr pf_t PF_EXEC = 0x04;

constexpr pf_t PF_ACCESSED = 0x10;
constexpr pf_t PF_DIRTY = 0x20;
constexpr pf_t PF_VALID = 0x40;
constexpr pf_t PF_ALLOCATED = 0x80; // RSW

constexpr pf_t PF_RW = PF_READ | PF_WRITE;
constexpr pf_t PF_RWX = PF_READ | PF_WRITE | PF_EXEC;
constexpr pf_t PF_RX = PF_READ | PF_EXEC;
constexpr pf_t PF_AD = PF_ACCESSED | PF_DIRTY;

constexpr pgidx_t INVALID_PAGE = -1;

using AccessSeq_t = std::vector<std::pair<pgidx_t, pf_t>>;

#else
#include CONFIG_WITH_CUSTOM_TYPES_HEADER
#endif

PGSUB_NAMESPACE_END

#ifndef __LIBPGSUB_H__
#define __LIBPGSUB_H__

// Default configuration


#ifndef CONFIG_WITH_NAMESPACE
#define CONFIG_WITH_NAMESPACE 1
#endif


/**
 * @brief Determines whether the OPT algorithm is enabled
 * @details Default is enabled.
 */
#ifndef CONFIG_ALGO_OPT_ENABLED
#define CONFIG_ALGO_OPT_ENABLED 1
#endif

#ifndef CONFIG_ALGO_FIFO_ENABLED
#define CONFIG_ALGO_FIFO_ENABLED 1
#endif

#ifndef CONFIG_ALGO_LRU_ENABLED
#define CONFIG_ALGO_LRU_ENABLED 1
#endif

#ifndef CONFIG_ALGO_CLOCK_ENABLED
#define CONFIG_ALGO_CLOCK_ENABLED 1
#endif


// Include algorithms

#if CONFIG_ALGO_OPT_ENABLED
#include "libpgsub/algo/OPT.hpp"
#endif

#if CONFIG_ALGO_FIFO_ENABLED
#include "libpgsub/algo/FIFO.hpp"
#endif

#if CONFIG_ALGO_LRU_ENABLED
#include "libpgsub/algo/LRU.hpp"
#endif

#if CONFIG_ALGO_CLOCK_ENABLED
#include "libpgsub/algo/Clock.hpp"
#endif

// Include simulation


#endif
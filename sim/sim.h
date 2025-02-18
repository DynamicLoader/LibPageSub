/**
 * @file start.h
 * @author DynamicLoader
 * @brief The C startup file for the simulator.
 * @version 0.1
 * @date 2024-11-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __START_H__
#define __START_H__

extern char __arg_start;
extern char __arg_end;

static int __parse_args(const char** argv, int max_argc)
{
    int argc = 0;
    char* str = &__arg_start;
    char* str_end = &__arg_end;
    while (argc < max_argc && str < str_end && *str) {
        while (*str && *str == ' ') {
            ++str;
        }
        if (*str) {
            argv[argc++] = str;
            while (*str && *str != ' ') {
                ++str;
            }
            if (*str) {
                *str++ = '\0';
            }
        }
    }
    return argc;
}

// Export the func as name, and call it with the raw argument (not parsed)
#define SIM_EXPORT_RAW(name, func)                               \
    int start()                                                  \
    {                                                            \
        const char* argv[] = { name, &__arg_start, &__arg_end }; \
        return func(3, argv);                                    \
    }
#endif

#define SIM_EXPORT(name, func, max_argc)                   \
    int start()                                            \
    {                                                      \
        const char* argv[max_argc + 1];                    \
        argv[0] = name;                                    \
        return func(1 + __parse_args(argv+1, max_argc), argv); \
    }

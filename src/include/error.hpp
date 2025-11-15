#pragma once
#include <cstdlib>

enum MessageLevel {
    LEVEL_CRIT = -2,
    LEVEL_ERROR = -1,
    LEVEL_MSG = 0,
    LEVEL_WARN = 1,
    LEVEL_INFO = 2,
    LEVEL_DEBUG = 3
};

#ifndef NO_COLORS

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#else

#define KNRM ""
#define KRED ""
#define KGRN ""
#define KYEL ""
#define KMAG ""
#define KCYN ""
#define KWTH ""

#endif

/*
 * Prints the message if the current verbosity level is high enough.
 * Returns whether the message was printed.
 */
bool print_message(MessageLevel level, int line, const char *file,
                   const char *format, ...);

#define CRIT(...)                                                   \
    do {                                                            \
        print_message(LEVEL_CRIT, __LINE__, __FILE__, __VA_ARGS__); \
        exit(-1);                                                   \
    } while (0)
#define ERROR(...) print_message(LEVEL_ERROR, __LINE__, __FILE__, __VA_ARGS__)
#define MSG(...) print_message(LEVEL_MSG, __LINE__, __FILE__, __VA_ARGS__)
#define WARN(...) print_message(LEVEL_WARN, __LINE__, __FILE__, __VA_ARGS__)
#define INFO(...) print_message(LEVEL_INFO, __LINE__, __FILE__, __VA_ARGS__)
#define DEBUG(...) print_message(LEVEL_DEBUG, __LINE__, __FILE__, __VA_ARGS__)

#define ASSERT(value, ...)                      \
    do {                                        \
        if (!(value))                           \
            ERROR("Assertion failed: " #value); \
    } while (0)

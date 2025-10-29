#include <config.hpp>
#include <cstdarg>
#include <cstdio>
#include <error.hpp>

bool print_message(MessageLevel level, int line, const char *file,
                   const char *format, ...) {
    bool do_print = (int)level <= Config::get().verbosity;
    va_list args;
    const char *title;

    if (!do_print)
        return false;

    va_start(args, format);
    switch (level) {
    case LEVEL_CRIT:
        title = "CRITICAL ERROR";
        break;
    case LEVEL_ERROR:
        title = "ERROR";
        break;
    case LEVEL_WARN:
        title = "WARNING";
        break;
    case LEVEL_INFO:
        title = "INFO";
        break;
    case LEVEL_DEBUG:
        title = "DEBUG";
        break;
    }

    switch (level) {
    case LEVEL_MSG:
        vfprintf(stdout, format, args);
        break;
    default:
        fprintf(stderr, "[%s | %s:%d]: ", title, file, line);
        vfprintf(stderr, format, args);
        break;
    }

    va_end(args);
    return true;
}

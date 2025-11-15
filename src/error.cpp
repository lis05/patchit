#include <config.hpp>
#include <cstdarg>
#include <cstdio>
#include <error.hpp>

bool print_message(MessageLevel level, int line, const char *file,
                   const char *format, ...) {
    bool        do_print = (int)level <= Config::get()->verbosity;
    va_list     args;
    const char *title;

    if (!do_print) {
        return false;
    }
	const char *color = KNRM;
	const char *clear = KNRM;

    va_start(args, format);
    switch (level) {
    case LEVEL_CRIT:
        title = "CRITICAL ERROR";
        break;
    case LEVEL_ERROR:
		color = KRED;
        title = "ERROR";
        break;
    case LEVEL_WARN:
		color = KYEL;
        title = "WARNING";
        break;
    case LEVEL_INFO:
		color = KCYN;
        title = "INFO";
        break;
    case LEVEL_DEBUG:
		color = KGRN;
        title = "DEBUG";
        break;
    }

    switch (level) {
    case LEVEL_MSG:
        vfprintf(stdout, format, args);
        break;
    default:
		fprintf(stderr, color);
        fprintf(stderr, "[%s | %s:%d]: ", title, file, line);
        vfprintf(stderr, format, args);
		fprintf(stderr, clear);
        break;
    }

    va_end(args);
    return true;
}

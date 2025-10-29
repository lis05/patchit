#include <getopt.h>

#include <config.hpp>
#include <cstdio>
#include <error.hpp>

static struct option const long_opts[] = {
    {"help", 0, 0, 'h'},
    {"verbose", 0, nullptr, 'v'},
    {"debug", 0, nullptr, 'd'},
    {nullptr, 0, nullptr, 0},
};

static const char *const short_opts = "-hvd";

void print_help() {
    printf(
        "Usage: printit [OPTIONS] ...\n"
		"\n"
		"  -h, --help               show this message\n"
		"  -v, --verbose            increase verbosity level (1 per v)\n"
		"  -d, --debug              set verbosity level to maximal (debug)\n"
	);
}

int main(int argc, char **argv) {
    char i;
    opterr = 0;
    while ((i = getopt_long(argc, argv, short_opts, long_opts, 0)) != -1) {
        DEBUG("Processing option -%c (%d)\n", i, (int)i);

        switch (i) {
        case 'h':
            print_help();
            return 0;
        case 'v':
            Config::get().verbosity++;
            break;
        case 'd':
            Config::get().verbosity = 10;
            break;
        case '?':
            if (optopt)
                CRIT("Unrecognized option: -%c\n", optopt);
            else if (argv[optind - 1])
                CRIT("Unrecognized option (possibly '%s')\n", argv[optind - 1]);
            else
                CRIT("Unrecognized option.\n");
            break;
		case 1:
			DEBUG("Processing other option '%s'\n", argv[optind - 1]);
			break;
        default:
            CRIT("Failed parsing options.\n");
        }
    }

	return 0;
}

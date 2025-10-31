#include <getopt.h>
#include <cstring>
#include <cstdio>

#include <config.hpp>
#include <error.hpp>

static struct option const long_opts[] = {
    {"help", 0, 0, 'h'},
    {"verbose", 0, nullptr, 'v'},
    {"debug", 0, nullptr, 'd'},
    {nullptr, 0, nullptr, 0},
};

static const char *const short_opts = "-hvd";

static const char *const COMMAND_CREATE = "create";
static const char *const COMMAND_APPLY = "apply";

static void print_help() {
    printf(
        "Usage: patchit [OPTIONS] COMMAND ...\n"
        "\n"
        "Options:\n"
        "  -h, --help               show this message\n"
        "  -v, --verbose            increase verbosity level (1 per v)\n"
        "  -d, --debug              set verbosity level to maximal (debug)\n"
        "\n"
        "Supported commands:\n"
        "  create                   create a new patch\n"
        "  apply                    apply the given patch\n");
}

int main(int argc, char **argv) {
    char short_option;
	const char *command;

    opterr = 0;
    while ((short_option = getopt_long(argc, argv, short_opts, long_opts, 0)) !=
           -1) {
        DEBUG("Processing option -%c (%d)\n", short_option, (int)short_option);

		switch (short_option) {
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
			command = argv[optind - 1];
            ASSERT(command);

			DEBUG("Processing command %s\n", command);

			if (!strcmp(command, COMMAND_CREATE)) {
				//
			}
			else if (!strcmp(command, COMMAND_APPLY)) {
				//
			}
            break;
        default:
            CRIT("Failed parsing options.\n");
        }
    }

    return 0;
}

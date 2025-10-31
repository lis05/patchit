#include <getopt.h>

#include <commands.hpp>
#include <config.hpp>
#include <cstdio>
#include <cstring>
#include <error.hpp>
#include <utility>

static struct option const long_opts[] = {
    {"help", 0, 0, 'h'},
    {"verbose", 0, nullptr, 'v'},
    {"debug", 0, nullptr, 'd'},
    {nullptr, 0, nullptr, 0},
};

static const char *const short_opts = "-hvd";

static const std::pair<const char *, CommandHandler> command_list[] = {
    {"create", do_command_create},
    {"apply", do_command_apply},
    {"inspect", do_command_inspect}};

static void print_help() {
    // clang-format off
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
        "  apply                    apply the given patch\n"
		"  inspect                  inspect contents of a patch\n"
	);
    // clang-format on
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
            if (optopt) {
                CRIT("Unrecognized option: -%c\n", optopt);
            } else if (argv[optind - 1]) {
                CRIT("Unrecognized option (possibly '%s')\n", argv[optind - 1]);
            } else {
                CRIT("Unrecognized option.\n");
            }
        case 1:
            command = argv[optind - 1];
            ASSERT(command);

            DEBUG("Processing command %s\n", command);

            for (auto &[name, func] : command_list) {
                if (!strcmp(command, name)) {
                    return func(argc - optind, argv + optind);
                }
            }

            CRIT("Unrecognized option: %s\n", command);
        default:
            CRIT("Failed parsing options.\n");
        }
    }

    CRIT("No command selected. Aborting.\n");
}

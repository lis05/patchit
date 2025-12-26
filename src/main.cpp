#include <getopt.h>

#include <commands.hpp>
#include <config.hpp>
#include <cstdio>
#include <cstring>
#include <error.hpp>
#include <utility>

static struct option const long_opts[] = {
    {"help", 0, nullptr, 'h'},    {"version", 0, nullptr, 'v'},
    {"verbose", 0, nullptr, 'V'}, {"info", 0, nullptr, 'I'},
    {"debug", 0, nullptr, 'D'},   {nullptr, 0, nullptr, 0},
};

static const char *const short_opts = "-hvVID";

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
		"  -v, --version            print version info\n"
        "  -V, --verbose            increase verbosity level (1 per V)\n"
		"  -I, --info               set verobsity level to info\n"
        "  -D, --debug              set verbosity level to debug (max)\n"
        "\n"
        "Supported commands:\n"
        "  create                   create a new patch\n"
        "  apply                    apply the given patch\n"
		"  inspect                  inspect contents of a patch\n"
	);
    // clang-format on
}

static void print_version() {
    const std::string &version = Config::get()->version;
    uint64_t           compatibility_version = Config::get()->compatibility_version;

    printf("Version: %s\n", version.c_str());

    if (compatibility_version != (uint64_t)-1) {
        printf("Compatibility version: %zu\n", (size_t)compatibility_version);
    } else {
        printf("Incompatible with all patches. Please reinstall.\n");
    }
}

int main(int argc, char **argv) {
    char        short_option;
    const char *command;

    opterr = 0;
    while ((short_option = getopt_long(argc, argv, short_opts, long_opts, 0)) !=
           -1) {
        DEBUG("Processing short option '%c' (%d)\n", short_option,
              (int)short_option);

        switch (short_option) {
        case 'h':
            print_help();
            return 0;
        case 'v':
            print_version();
            return 0;
        case 'V':
            Config::get()->verbosity++;
            break;
        case 'I':
            Config::get()->verbosity = 2;
            break;
        case 'D':
            Config::get()->verbosity = 3;
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

            INFO("Received command: %s\n", command);

            for (auto &[name, func] : command_list) {
                if (!strcmp(command, name)) {
                    argv[0] = strdup(command);  // leaks memory but its ok
                    return func(argc - optind + 1, argv + optind - 1);
                }
            }

            CRIT("Unrecognized option: %s\n", command);
        default:
            CRIT("Failed parsing options.\n");
        }
    }

    MSG("No command selected. Aborting.\n");
    return -1;
}

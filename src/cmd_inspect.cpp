#include <getopt.h>

#include <commands.hpp>
#include <diff.hpp>
#include <error.hpp>
#include <patch.hpp>
#include <util.hpp>
#include <utility>

static struct option const long_opts[] = {{"help", 0, nullptr, 'h'},
                                          {"verbose", 0, nullptr, 'V'}};

static const char *const short_opts = "-hV";

static void print_help() {
    // clang-format off
	printf(
		"Usage: inspect [FLAGS] PATCHFILE\n"
		"\n"
		"Flags:\n"
		"  -h, --help                 show this message\n"
		"  -V, --verbose              increase verbosity level\n"
	);
    // clang-format on
}

int do_command_inspect(int argc, char **argv) {
    INFO("Running inspect command.\n");
    for (int i = 0; i < argc; i++) {
        DEBUG("Got argv[i]: %s\n", argv[i]);
    }

    int         r = -1;
    char        short_option;
    int         verbosity = 0;
    const char *patchfile = NULL;
    Patch       p;

    optind = 1;
    opterr = 0;
    while ((short_option = getopt_long(argc, argv, short_opts, long_opts, 0)) !=
           -1) {
        DEBUG("Processing short option '%c' (%d)\n", short_option,
              (int)short_option);

        switch (short_option) {
        case 'h':
            print_help();
            return 0;
        case 'V':
            verbosity++;
            break;
        case 1:
            patchfile = argv[optind - 1];
            INFO("Patchfile specified: %s\n", patchfile);
            goto inspect;
        case '?':
            handle_unknown_option(optind, optopt, argv);
            break;
        default:
            CRIT("Failed to parse options.\n");
        }
    }

    ERROR("Please specify patchfile.\n");
    return -1;

inspect:
    if ((r = p.load_from_file(patchfile))) {
        ERROR("Failed to inspect the patch.\n");
        return r;
    }

    p.inspect_contents(verbosity);

    return r;
}

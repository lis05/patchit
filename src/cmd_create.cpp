#include <commands.hpp>
#include <diff.hpp>
#include <patch.hpp>
#include <cstdio>
#include <cstring>
#include <error.hpp>
#include <utility>
#include <getopt.h>

static struct option const long_opts[] = {
	{"help", 0, nullptr, 'h'},
	{"modify", 0, nullptr, 'M'},
	{"compressor", 0, nullptr, 'c'},
	{"diff", 0, nullptr, 'd'},
	{nullptr, 0, nullptr, 0}
};

static const char *const short_opts = "-hMc:e";

static void print_help() {
	// clang-format off
	printf(
		"Usage: create PATCHFILE INSTRUCTIONS\n"
		"\n"
		"Instructions will be stored in the same order\n"
		"    that they were declared here.\n"
		"\n"
		"Instructions with their respective flags:\n"
		"\n"
		"File modification:\n"
		"  -M, --modify FLAGS SOURCEFILE DESTFILE\n"
		"                             Append a file modification instruction.\n"
		"Flags:\n"
		"  -e                         Create an empty file if the target\n"
		"                                 does not exist.\n"
		"  -d, --diff       DIFF      Use the selected diff method.\n"
		"                                 Supported diffs: default\n"
		"  -c, --compressor COMP      Use the selected compression method.\n"
		"                                 Supported compressors: default\n"
	);
	// clang-format on
}

int do_command_create(int argc, char **argv) {
	for (int i = 0; i < argc; i++) {
		DEBUG("Got argv[i]: %s\n", argv[i]);
	}

	char short_option;
	const char *patchfile = NULL;

	Patch p;

	optind = 1;
	opterr = 0;
	while ((short_option = getopt_long(argc, argv, short_opts, long_opts, 0)) !=
		-1) {
		DEBUG("Processing option -%c (%d)\n", short_option, (int)short_option);

		switch (short_option) {
		case 'h':
			print_help();
			return 0;
		case '?':
            if (optopt) {
                CRIT("Unrecognized option: -%c\n", optopt);
            } else if (argv[optind - 1]) {
                CRIT("Unrecognized option (possibly '%s')\n", argv[optind - 1]);
            } else {
                CRIT("Unrecognized option.\n");
            }
        case 1:
            /*command = argv[optind - 1];
            ASSERT(command);

            DEBUG("Processing command %s\n", command);

            for (auto &[name, func] : command_list) {
                if (!strcmp(command, name)) {
                    return func(argc - optind, argv + optind);
                }
            }

            CRIT("Unrecognized option: %s\n", command);*/
			CRIT("");
        default:
            CRIT("Failed parsing options.\n");
        }
	}
}

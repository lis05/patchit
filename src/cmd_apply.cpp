#include <getopt.h>
#include <unistd.h>

#include <commands.hpp>
#include <cstdio>
#include <cstring>
#include <diff.hpp>
#include <error.hpp>
#include <patch.hpp>
#include <util.hpp>
#include <utility>

static struct option const long_opts[] = {{"help", 0, nullptr, 'h'},
                                          {nullptr, 0, nullptr, 0}};

static const char *const short_opts = "-h";

static void print_help() {
    // clang-format off
	printf(
		"Usage: apply PATCHFILE DESTPATH\n"
		"\n"
		"Apply the given patchfile at the provided path.\n"
	);
    // clang-format on
}

int do_command_apply(int argc, char **argv) {
    INFO("Running apply command.\n");
    for (int i = 0; i < argc; i++) {
        DEBUG("argv[%d] = %s\n", i, argv[i]);
    }

    int         r = -1;
    char        short_option;
    const char *patchfile = NULL;
    const char *destpath = NULL;
    char       *oldwd;
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
        case 1:
            if (!patchfile) {
                patchfile = argv[optind - 1];
                INFO("Patchfile specified: %s\n", patchfile);
                break;
            } else if (!destpath) {
                destpath = argv[optind - 1];
                INFO("Destpath specified: %s\n", destpath);
                goto apply;
            }
        case '?':
            handle_unknown_option(optind, optopt, argv);
            break;
        default:
            CRIT("Failed to parse options.\n");
            return -1;
        }
    }

    ERROR("Please specify patchfile and destpath.\n");
    return -1;

apply:
    if ((r = p.load_from_file(patchfile))) {
        ERROR("Failed to apply the patch.\n");
        return r;
    }

    oldwd = getcwd(NULL, 0);
    if (!oldwd) {
        ERROR("Failed getcwd: %s\n", strerror(errno));
        return -1;
    }

    if (chdir(destpath)) {
        ERROR("Failed to change current working directory to %s: %s\n", destpath,
              strerror(errno));
        return -1;
    }
    INFO("Changed CWD to %s\n", destpath);

    INFO("Applying the patch...\n");
    r = p.apply();

    if (chdir(oldwd)) {
        ERROR("Failed chdir(%s): %s\n", oldwd, strerror(errno));
        return -1;
    }

    if (!r) {
        MSG("Applied the patch successfully.\n");
    } else {
        ERROR("Failed to apply the patch. (%d)\n", r);
    }

    return r;
}

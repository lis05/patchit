#include <getopt.h>

#include <commands.hpp>
#include <config.hpp>
#include <cstdio>
#include <cstring>
#include <diff.hpp>
#include <error.hpp>
#include <patch.hpp>
#include <util.hpp>
#include <utility>

static struct option const long_opts[] = {{"help", 0, nullptr, 'h'},
                                          {"modify", 0, nullptr, 'M'},
                                          {"compressor", 0, nullptr, 'c'},
                                          {"diff", 0, nullptr, 'd'},
                                          {nullptr, 0, nullptr, 0}};

static const char *const short_opts = "-hMc:d:pe";

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
		"  -p                         Create all the necessary subdirectories\n"
		"                                 if they do not exist.\n"
		"  -e                         Create an empty file if the target does\n"
		"                                 not exist before applying the patch.\n"
		"  -d, --diff       DIFF      Use the selected diff method.\n"
		"                                 Supported diffs: default\n"
		"  -c, --compressor COMP      Use the selected compression method.\n"
		"                                 Supported compressors: default\n"
	);
    // clang-format on
}

int do_create_entity_modification(int argc, char **argv, Patch &p) {
    INFO("Handling entity modification instruction.\n");
    for (int i = 0; i < argc; i++) {
        DEBUG("argv[%d] = %s\n", i, argv[i]);
    }
    char short_option;

    std::shared_ptr<Instruction> ins;
    std::shared_ptr<Diff>        diff = std::make_shared<SystemDiff>();
    bool                         create_empty_file_if_not_exists = false;
    bool                         create_subdirectories = false;
    char                        *from_file = NULL, *to_file = NULL;
    Config::get()->compressor = PlainCompressor::get();

    while ((short_option = getopt_long(argc, argv, short_opts, long_opts, 0)) !=
           -1) {
        DEBUG("Processing short option '%c' (%d)\n", short_option,
              (int)short_option);
        switch (short_option) {
        case 'p':
            INFO("Selected create_subdirectories = true\n");
            create_subdirectories = true;
            break;
        case 'e':
            INFO("Selected create_empty_file_if_not_exists = true\n");
            create_empty_file_if_not_exists = true;
            break;
        case 'd':
            INFO("Selected diff: %s\n", optarg);
            if (!strcmp(optarg, "default")) {
                diff.reset(new SystemDiff());
            } else {
                ERROR("Unrecognized diff selected: %s\n", optarg);
                return -1;
            }
            break;
        case 'c':
            INFO("Selected compressor: %s\n", optarg);
            if (!strcmp(optarg, "default")) {
                Config::get()->compressor = PlainCompressor::get();
            } else {
                ERROR("Unrecognized compressor selected: %s\n", optarg);
                return -1;
            }
            break;
        case '?':
            handle_unknown_option(optind, optopt, argv);
            return -1;
        case 1:
            if (!from_file) {
                from_file = argv[optind - 1];
                INFO("From file: %s\n", from_file);
            } else if (!to_file) {
                to_file = argv[optind - 1];
                INFO("To file: %s\n", to_file);
                goto create;
            }
            break;
        default:
            CRIT("Failed to parse options.\n");
            return -1;
        }
    }

    ERROR("Please specify source and destination files.\n");
    return -1;

create:
    if (diff->from_files(from_file, to_file)) {
        ERROR(
            "Failed to create an entity modification instruction: diff creation has "
            "failed.\n");
        return -1;
    }

    ins.reset(new EntityModifyInstruction(
        create_subdirectories, create_empty_file_if_not_exists, from_file, diff));
    p.append(ins);
    INFO("Successfully created new entity modify instruction: %s -> %s.\n",
         from_file, to_file);
    return 0;
}

int do_command_create(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        DEBUG("Got argv[i]: %s\n", argv[i]);
    }

    int         r = -1;
    char        short_option;
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
        case 'M':
            if (!patchfile) {
                ERROR("Patchfile was not specified.\n");
                return -1;
            }

            if ((r = do_create_entity_modification(argc, argv, p))) {
                return r;
            }
            break;
        case '?':
            handle_unknown_option(optind, optopt, argv);
            return -1;
        case 1:
            if (patchfile) {
                ERROR("Patchfile already specified.\n");
                return -1;
            }
            INFO("Destination patchfile: %s\n", argv[optind - 1]);
            patchfile = argv[optind - 1];
            break;
        default:
            ERROR("Failed to parse options.\n");
            break;
        }
    }

    r = p.write_to_file(patchfile);

    if (!r) {
        MSG("Created a patch successfully.\n");
    } else {
        ERROR("Failed to create a patch (%d).\n", r);
    }
    return r;
}

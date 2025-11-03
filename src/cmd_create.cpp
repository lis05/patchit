#include <commands.hpp>
#include <patch.hpp>
#include <diff.hpp>

int do_command_create(int argc, char **argv) {
	SystemDiff diff;
	diff.from_files("src/config.cpp", "src/error.cpp");
}

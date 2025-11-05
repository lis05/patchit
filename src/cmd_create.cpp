#include <commands.hpp>
#include <diff.hpp>
#include <patch.hpp>

int do_command_create(int argc, char **argv) {
    SystemDiff diff;
    diff.from_files("test/file1", "test/file2");
	diff.apply("test/file1");
}

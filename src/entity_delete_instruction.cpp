#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <error.hpp>
#include <filesystem>
#include <patch.hpp>
#include <util.hpp>
#include <utility>

EntityDeleteInstruction::EntityDeleteInstruction() {
	this->signature = ENTITY_DELETE;
}

EntityDeleteInstruction::EntityDeleteInstruction(
    bool delete_recursively_if_directory, const std::string &target) {
    this->signature = ENTITY_DELETE;
    this->delete_recursively_if_directory = delete_recursively_if_directory;
    this->target = target;
}

int EntityDeleteInstruction::apply() {
    INFO("Applying EntityDeleteInstruction.\n");

    std::string command = "rm -v --interactive=never --preserve-root=all";
    if (delete_recursively_if_directory) {
        command += " -r";
    }

    command += " " + target;
    INFO("Deleting target %s using '%s'\n", target.c_str(), command.c_str());

    int r = system(command.c_str());
    if (r == -1 || WEXITSTATUS(r) != 0) {
        ERROR("Failed to execute '%s'\n", command.c_str());
        return -1;
    }

    INFO("Applied deletion successfully to %s\n", target.c_str());
    return 0;
}

std::vector<std::byte> EntityDeleteInstruction::binary_representation() {
    std::vector<std::byte> res;

    res.push_back(std::byte{delete_recursively_if_directory});

    for (size_t i = 0; i < target.size(); i++) {
        res.push_back(std::byte(target[i]));
    }
    res.push_back(std::byte{0});

    return res;
}

int EntityDeleteInstruction::from_binary_representation(
    const std::vector<std::byte> &data) {
    INFO("Restoring EntityDeleteInstruction\n");
    if (data.size() < 2) {
        ERROR("Corrupted data: not enough bytes\n");
        return -1;
    }

    delete_recursively_if_directory = (bool)data[0];

    INFO("  delete_recursively_if_directory flag: %d\n",
         (int)delete_recursively_if_directory);

    if (std::find(data.begin() + 1, data.end(), std::byte{0}) == data.end()) {
        ERROR("Invalid source file: no NULL byte\n");
        return -1;
    }

    target = std::string((char *)(data.data()) + 1);
    INFO("  target: %s\n", target.c_str());

    return 0;
}

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

EntityMoveInstruction::EntityMoveInstruction() {
    this->signature = ENTITY_MOVE;
}

EntityMoveInstruction::EntityMoveInstruction(bool create_subdirectories,
                                             bool override_if_already_exists,
                                             const std::string &move_from,
                                             const std::string &move_to) {
    this->signature = ENTITY_MOVE;
    this->create_subdirectories = create_subdirectories;
    this->override_if_already_exists = override_if_already_exists;
    this->move_from = move_from;
    this->move_to = move_to;
}

int EntityMoveInstruction::apply() {
    INFO("Applying EntityMoveInstruction.\n");

    struct stat sb_src, sb_dest;
    if (stat(move_from.c_str(), &sb_src)) {
        ERROR("Cannot relocate %s: %s\n", move_from.c_str(), strerror(errno));
        return -1;
    }

    if (!S_ISREG(sb_src.st_mode)) {
        ERROR("Cannot relocate: %s is not a regular file.\n", move_from.c_str());
        return -1;
    }

    if (!(sb_src.st_mode & S_IRUSR)) {
        ERROR("Cannot relocate: does not have permission to read %s\n",
              move_from.c_str());
        return -1;
    }

    bool exists = !stat(move_to.c_str(), &sb_dest);

    INFO("Destination exists: %d\n", (int)exists);
    INFO("Override flag: %d\n", (int)override_if_already_exists);

    if (!exists) {
        if (create_subdirectories) {
            std::filesystem::path p = move_to;
            p = p.remove_filename();
            std::string path = p;
            INFO("Creating subdirectories: '%s' (due to a flag)...\n", path.c_str());
            char *dirpath = strdup(path.c_str());
            if (dirpath) {
                mkdirr(dirpath, 0777);
            } else {
                ERROR("strdup: out of memory\n");
                return -1;
            }
            free(dirpath);
        }
    }

    if (!exists || override_if_already_exists) {
        INFO("Moving %s -> %s\n", move_from.c_str(), move_to.c_str());
        std::vector<std::byte> data;
        if (open_and_read_entire_file(move_from.c_str(), data) ||
            open_and_write_entire_file(move_to.c_str(), data)) {
            ERROR("Failed moving data between files.\n");
            return -1;
        }
        if (remove(move_from.c_str())) {
            ERROR("Failed to remove the source file %s: %s\n", move_from.c_str(),
                  strerror(errno));
            return -1;
        }
    } else {
        INFO("Not relocating.\n");
    }

	INFO("Applied relocation successfully to %s\n", move_from.c_str());

    return 0;
}

std::vector<std::byte> EntityMoveInstruction::binary_representation() {
    std::vector<std::byte> res;

    res.push_back(std::byte{override_if_already_exists});
    res.push_back(std::byte{create_subdirectories});

    for (size_t i = 0; i < move_from.size(); i++) {
        res.push_back(std::byte(move_from[i]));
    }
    res.push_back(std::byte{0});

    for (size_t i = 0; i < move_to.size(); i++) {
        res.push_back(std::byte(move_to[i]));
    }
    res.push_back(std::byte{0});

    return res;
}

int EntityMoveInstruction::from_binary_representation(
    const std::vector<std::byte> &data) {
	INFO("Restoring EntityMoveInstruction\n");
    if (data.size() < 4) {
        ERROR("Corrupted data: not enough bytes\n");
        return -1;
    }

    override_if_already_exists = (bool)data[0];
    create_subdirectories = (bool)data[1];

	INFO("  override flag: %d\n", (int)override_if_already_exists);
	INFO("  create_subdirectories flag: %d\n", (int)create_subdirectories);

    if (std::find(data.begin() + 2, data.end(), std::byte{0}) == data.end()) {
        ERROR("Invalid source file: no NULL byte\n");
        return -1;
    }

    move_from = std::string((char *)(data.data()) + 2);
	INFO("  move_from: %s\n", move_from.c_str());

    int where = move_from.size() + 3;

    if (std::find(data.begin() + where, data.end(), std::byte{0}) == data.end()) {
        ERROR("Invalid destination file: no NULL byte\n");
        return -1;
    }

    move_to = std::string((char *)(data.data()) + where);
	INFO("  move_to: %s\n", move_to.c_str());

    return 0;
}

#include <cstring>
#include <error.hpp>
#include <patch.hpp>
#include <util.hpp>

static const char *const SIGNATURE = "__PATCHIT__";

std::shared_ptr<Instruction> Instruction::from_signature(uint8_t signature) {
    std::shared_ptr<Instruction> res;
    switch (signature) {
    case Instruction::ENTITY_MOVE:
        res.reset(new EntityMoveInstruction());
        break;
    case Instruction::ENTITY_DELETE:
        res.reset(new EntityDeleteInstruction());
        break;
    case Instruction::ENTITY_MODIFY:
        INFO("Instruction signature recognized: ENTITY_MODIFY\n");
        res.reset(new EntityModifyInstruction());
        break;
        // case Instruction::ENTITY_CHANGE_PERMISSIONS:
        //	res.reset(new EntityChangePermissionsInstruction());
        //	break;
    }

    if (!res) {
        WARN("Unrecognized instruction signature: %d\n", (int)signature);
    }
    return res;
}

int Patch::apply() {
    INFO("Applying patch...\n");
    for (auto i : instructions) {
        if (i->apply()) {
            ERROR("Failed to apply patch.\n");
            return -1;
        }
    }

    return 0;
}

void Patch::append(std::shared_ptr<Instruction> instruction) {
    this->instructions.push_back(instruction);
}

/*
 * Binary representation:
 *
 * SIGNATURE(with NULL byte)
 * compatibility_version (uint64, least to most significant)
 * number_of_instructions (uint64_t, least to most significant)
 * len_I1 (uint64_t, ...)
 * I1_signature (1byte)
 * I1
 * len_I2 (uint64_t, ...)
 * I2_signature (1byte)
 * I2
 * ...
 */

int Patch::write_to_file(const std::string &file) {
    INFO("Writing patch to file: %s\n", file.c_str());
    std::vector<std::byte> data;

    for (char *ptr = (char *)SIGNATURE; *ptr != 0; ptr++) {
        data.push_back((std::byte)*ptr);
    }

    data.push_back(std::byte{0});

    store_uint64_t(Patch::compatibility_version, data);

    store_uint64_t(instructions.size(), data);

    for (auto i : instructions) {
        const std::vector<std::byte> repr = i->binary_representation();
        store_uint64_t(repr.size(), data);
        data.push_back((std::byte)i->signature);
        for (std::byte b : repr) {
            data.push_back(b);
        }
    }

    return open_and_write_entire_file(file.c_str(), data);
}

int Patch::load_from_file(const std::string &file) {
    INFO("Loading patch from file: %s\n", file.c_str());
    std::vector<std::byte> data;

    if (open_and_read_entire_file(file.c_str(), data)) {
        ERROR("Failed to load patch %s\n", file.c_str());
        return -1;
    }
    std::vector<std::byte>::iterator it = data.begin();

    if (data.size() < sizeof(SIGNATURE) ||
        memcmp(data.data(), SIGNATURE, strlen(SIGNATURE))) {
        ERROR("Failed to load patch %s: invalid signature.\n", file.c_str());
        return -1;
    }
    it += strlen(SIGNATURE);

    if (it >= data.end() || *it != std::byte{0}) {
        ERROR("Failed to load patch %s: invalid signature separator.\n",
              file.c_str());
        return -1;
    }
    it++;

    uint64_t compatibility_version;
    if (restore_uint64_t(it, data.end(), compatibility_version)) {
        ERROR("Failed to load patch %s: invalid compatibility version.\n",
              file.c_str());
        return -1;
    }

    if (compatibility_version != Patch::compatibility_version) {
        ERROR(
            "Failed to load patch %s: compatibility version differs: found %zu, "
            "must be %zu\n",
            (size_t)compatibility_version, (size_t)Patch::compatibility_version);
    }

    uint64_t count;
    if (restore_uint64_t(it, data.end(), count)) {
        ERROR("Failed to load patch %s: invalid number of instructions.\n",
              file.c_str());
        return -1;
    }
    INFO("Patch contains %zu instructions.\n", count);

    std::vector<std::byte> repr;
    uint64_t               len;

    while (count--) {
        if (restore_uint64_t(it, data.end(), len)) {
            ERROR("Failed to load patch %s: invalid instruction size.\n",
                  file.c_str());
            return -1;
        }

        std::shared_ptr<Instruction> instruction;
        if (it == data.end() ||
            !(instruction = Instruction::from_signature((uint8_t)*it))) {
            ERROR("Failed to load patch %s: invalid instruction signature.\n",
                  file.c_str());
            return -1;
        }
        it++;

        repr.clear();
        try {
            repr.reserve(len);
        } catch (...) {
            ERROR("Failed to load patch %s: Likely out of memory.\n", file.c_str());
            return -1;
        }

        while (len--) {
            if (it == data.end()) {
                ERROR("Failed to load patch %s: truncated instruction.\n",
                      file.c_str());
                return -1;
            }
            repr.push_back(*it);
            it++;
        }

        if (instruction->from_binary_representation(repr)) {
            ERROR("Failed to load patch %s: corrupted instruction.\n", file.c_str());
            return -1;
        }

        append(instruction);
    }
    INFO("Loaded %zu instructions successfully.\n", instructions.size());
    return 0;
}

void Patch::inspect_contents(int verbosity) {
    MSG("compatibility version: %zu\n", (size_t)Patch::compatibility_version);
    MSG("contains: %zu instructions\n", instructions.size());

    if (verbosity < 1) {
        return;
    }

    MSG("instructions:\n");
    for (size_t i = 0; i < instructions.size(); i++) {
        MSG("  %zu. ", i + 1);

        const Instruction *const       ins = instructions[i].get();
        const EntityModifyInstruction *emIns;

        switch (ins->signature) {
        case Instruction::ENTITY_MODIFY:
            MSG("entity modification\n");
            emIns = (const EntityModifyInstruction *)ins;
            if (verbosity >= 2) {
                MSG("    target: %s\n", emIns->target.c_str());
                if (verbosity >= 3) {
                    MSG("    flags: ");
                    if (emIns->create_subdirectories) {
                        MSG("create subdirectories, ");
                    }
                    if (emIns->create_empty_file_if_not_exists) {
                        MSG("create empty file if not exists, ");
                    }
                    MSG("\n");
                }
            }
            break;
        }
    }
}

#include <patch.hpp>
#include <utility>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>
#include <cstdio>
#include <error.hpp>
#include <algorithm>

EntityModifyInstruction::EntityModifyInstruction(
    bool create_empty_file_if_not_exists, std::string target, std::shared_ptr<Diff> diff) {
	this->signature = ENTITY_MODIFY;
    this->create_empty_file_if_not_exists = create_empty_file_if_not_exists;
	this->target = target;
    this->diff = diff;
}

int EntityModifyInstruction::apply() {
	struct stat sb;
	FILE *fd = NULL;

	if (stat(target.c_str(), &sb)) {
		if (create_empty_file_if_not_exists) {
			if (!(fd = std::fopen(target.c_str(), "w")) || !std::fclose(fd)) {
				ERROR("Failed to create %s: %s\n", target.c_str(), strerror(errno));
				return -1;
			}
			return 0;
		}
		ERROR("Failed to stat %s: %s\n", target.c_str(), strerror(errno));
		return -1;
	}

	if (!S_ISREG(sb.st_mode)) {
		ERROR("Cannot apply modification: %s is not a regular file.\n", target.c_str());
		return -1;
	}

	if (!(sb.st_mode & S_IRUSR)) {
		ERROR("Cannot apply modification: does not have permission to read %s\n", target.c_str());
		return -1;
	}

	if (!(sb.st_mode & S_IWUSR)) {
		ERROR("Cannot apply modification: does not have permission to write %s\n", target.c_str());
		return -1;
	}

	if (diff->apply(target)) {
		ERROR("Failed to apply modification to %s\n", target.c_str());
		return -1;
	}

	INFO("Applied modification to %s\n", target.c_str());
	return 0;
}

std::vector<std::byte> EntityModifyInstruction::binary_representation() {
	std::vector<std::byte> data;
	std::vector<std::byte> diff_data = diff->binary_representation();

	data.reserve(1 + (target.size() + 1) + diff_data.size());

	data.push_back(std::byte{diff->signature});

	for (char c: target) {
		data.push_back((std::byte)c);
	}
	data.push_back(std::byte{0});

	data.push_back((std::byte)this->create_empty_file_if_not_exists);

	for (std::byte byte: diff_data) {
		data.push_back(byte);
	}

	return data;
}

int EntityModifyInstruction::from_binary_representation(
    const std::vector<std::byte> &data) {

	if (data.empty()) {
		WARN("Empty instruction.\n");
		return 0;
	}

	uint8_t signature = (uint8_t)data[0];

	diff = Diff::from_signature(signature);

	if (!diff) {
		ERROR("Invalid diff signature: %d\n", (int)signature);
		return -1;
	}

	if (std::find(data.begin() + 1, data.end(), std::byte{0}) == data.end()) {
		ERROR("Invalid diff target: no NULL byte.\n");
		return -1;
	}

	target = std::string((char*)(data.data()) + 1);
	INFO("Instruction's target: %s\n", target.c_str());

	if (data.size() < 1 + target.size() + 1) {
		ERROR("Invalid diff: no flags.\n");
		return -1;
	}

	this->create_empty_file_if_not_exists = (bool)data[1 + target.size() + 1];

	std::vector<std::byte> data_copy = data;
	data_copy.erase(data_copy.begin(), data_copy.begin() + 1 + target.size() + 2);

	return diff->from_binary_representation(data_copy);
}


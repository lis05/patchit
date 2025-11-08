#include <patch.hpp>
#include <util.hpp>

static const char *const SIGNATURE = "__PATCHIT__";

int Patch::apply() {

}

void Patch::append(std::shared_ptr<Instruction> instruction) {

}

/*
 * Binary representation:
 *
 * SIGNATURE(with NULL byte)
 * number-of-instructions(string, with NULL byte)
 * I1
 * I2
 * ...
 */

int Patch::write_to_file(const std::string &file) {
	std::vector<std::byte> data;

	for (char *ptr = (char*)SIGNATURE; *ptr != 0; ptr++) {
		data.push_back((std::byte)*ptr);
	}

	data.push_back(std::byte{0});

	for (char c: std::to_string(instructions.size())) {
		data.push_back((std::byte)c);
	}

	data.push_back(std::byte{0});

	for (auto i: instructions) {
		for (std::byte b: i->binary_representation()) {
			data.push_back(b);
		}
	}

	return open_and_write_entire_file(file.c_str(), data);
}

int Patch::load_from_file(const std::string &file) {}

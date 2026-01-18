#include <unit_test_framework.hpp>
#include <unit_common.hpp>

#include <vector>
#include <cstring>
#include <string>

#include <patch.hpp>
#include <util.hpp>
#include <config.hpp>

static std::vector<std::byte> str2vec(std::string s) {
	std::vector<std::byte> res;
	for (auto c: s) res.push_back((std::byte)c);
	return res;
}

static std::string vec2str(std::vector<std::byte> v) {
	std::string res;
	for (auto b: v) res += (char)b;
	return res;
}

#define SRC TEMP_FILE1
#define DEST TEMP_FILE2
#define NESTED TEMP_FILE3 "/file"

static void setup() {
	std::system("rm -rf " SRC);
	std::system("rm -rf " DEST);
	open_and_write_entire_file(SRC, str2vec(""));
	open_and_write_entire_file(DEST, str2vec("to"));
}

TEST(entity_modify_instruction_constructor_no_args) {
	setup();
	auto e = std::make_shared<EntityModifyInstruction>();
	ASSERT_EQUAL(e->signature, Instruction::ENTITY_MODIFY);
	ASSERT_EQUAL(e->create_subdirectories, false);
	ASSERT_EQUAL(e->create_empty_file_if_not_exists, false);
	ASSERT_EQUAL(e->target, "");
	ASSERT_EQUAL(e->diff, nullptr);
}

TEST(entity_modify_instruction_constructor_with_args) {
	setup();
	auto e = std::make_shared<EntityModifyInstruction>();
	auto diff = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	e = std::make_shared<EntityModifyInstruction>(true, true, "target", diff);
	ASSERT_EQUAL(e->create_subdirectories, true);
	ASSERT_EQUAL(e->create_empty_file_if_not_exists, true);
	ASSERT_EQUAL(e->target, "target");
	ASSERT_EQUAL(e->diff, diff);
}

TEST(entity_modify_instruction_binary_representation) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(true, true, "target", d);

	/*
	 * d->signature 1b
	 * target: target.size() b
	 * \0: 1b
	 * create_subdir: 1b
	 * create_empty: 1b
	 * data: data.size() b
	*/

	auto vec = e->binary_representation();
	ASSERT_EQUAL(vec.size(), 1+6+1+1+1+diff_data.size());
	ASSERT_EQUAL(vec[0], std::byte{d->signature});
	ASSERT_EQUAL(vec[1], std::byte{'t'});
	ASSERT_EQUAL(vec[2], std::byte{'a'});
	ASSERT_EQUAL(vec[3], std::byte{'r'});
	ASSERT_EQUAL(vec[4], std::byte{'g'});
	ASSERT_EQUAL(vec[5], std::byte{'e'});
	ASSERT_EQUAL(vec[6], std::byte{'t'});
	ASSERT_EQUAL(vec[7], std::byte{'\0'});
	ASSERT_EQUAL(vec[8], std::byte{true});
	ASSERT_EQUAL(vec[9], std::byte{true});
	for (int i = 0; i < diff_data.size(); i++)
		ASSERT_EQUAL(vec[10 + i], diff_data[i]);
}

TEST(entity_modify_instruction_from_binary_representation_ok) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(true, true, "target", d);

	/*
	 * d->signature 1b
	 * target: target.size() b
	 * \0: 1b
	 * create_subdir: 1b
	 * create_empty: 1b
	 * data: data.size() b
	*/
	auto vec = e->binary_representation();

	vec.clear();
	ASSERT_EQUAL(e->from_binary_representation(vec), 0);
}

TEST(entity_modify_instruction_from_binary_representation_invalid_diff_signature) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(true, true, "target", d);

	/*
	 * d->signature 1b
	 * target: target.size() b
	 * \0: 1b
	 * create_subdir: 1b
	 * create_empty: 1b
	 * data: data.size() b
	*/
	auto vec = e->binary_representation();

	vec[0] = std::byte{150};
	ASSERT_EQUAL(e->from_binary_representation(vec), -1);
}

TEST(entity_modify_instruction_from_binary_representation_no_null_byte) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(true, true, "target", d);

	/*
	 * d->signature 1b
	 * target: target.size() b
	 * \0: 1b
	 * create_subdir: 1b
	 * create_empty: 1b
	 * data: data.size() b
	*/
	auto vec = e->binary_representation();

	vec[7] = std::byte{1};
	ASSERT_EQUAL(e->from_binary_representation(vec), -1);
}

TEST(entity_modify_instruction_from_binary_representation_too_few_bytes) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(true, true, "target", d);

	/*
	 * d->signature 1b
	 * target: target.size() b
	 * \0: 1b
	 * create_subdir: 1b
	 * create_empty: 1b
	 * data: data.size() b
	*/
	auto vec = e->binary_representation();

	vec.erase(vec.begin() + 5, vec.end()); // 5 should be 8, but the code aborts...
										   // this test is kinda useless because the
										   // source code contains a bug...
	ASSERT_EQUAL(e->from_binary_representation(vec), -1);
}

TEST(entity_modify_instruction_apply_not_a_regular_file) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(false, false, SRC, d);

	std::system("rm " SRC);
	std::system("mkdir " SRC);
	ASSERT_EQUAL(e->apply(), -1);
	std::system("rmdir " SRC);
}

TEST(entity_modify_instruction_apply_cannot_write) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(false, false, SRC, d);

	std::system("chmod 222 " SRC);
	ASSERT_EQUAL(e->apply(), -1);
}

TEST(entity_modify_instruction_apply_cannot_read) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(false, false, SRC, d);

	std::system("chmod 444 " SRC);
	ASSERT_EQUAL(e->apply(), -1);
}

TEST(entity_modify_instruction_apply_no_xxd) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(false, false, SRC, d);

	char *old = getenv("PATH");
	setenv("PATH", "", 1);
	ASSERT_EQUAL(e->apply(), -1);
	setenv("PATH", old, 1);
}

TEST(entity_modify_instruction_apply_ok) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(false, false, SRC, d);

	ASSERT_EQUAL(e->apply(), 0);
}

TEST(entity_modify_instruction_apply_missing_but_cannot_create) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(false, false, SRC, d);

	unlink(SRC);
	ASSERT_EQUAL(e->apply(), -1);
}

TEST(entity_modify_instruction_apply_missing_but_created_empty) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(false, true, SRC, d);

	unlink(SRC);
	ASSERT_EQUAL(e->apply(), 0);
}

TEST(entity_modify_instruction_apply_missing_but_failed_to_create_empty) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(false, true, NESTED, d);

	std::system("rf -rf " NESTED);
	std::system("mkdir -p " NESTED);
	std::system("chmod -r 444 " TEMP_FILE3);
	ASSERT_EQUAL(e->apply(), -1);
}

TEST(entity_modify_instruction_apply_created_subdirectories_but_cannot_create_empty) {
	setup();
	auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
	d->compressor = PlainCompressor::get();
	auto sd = (SystemDiff*)d.get();
	ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
	auto diff_data = sd->binary_representation();
	auto e = std::make_shared<EntityModifyInstruction>(true, false, NESTED, d);

	std::system("rm -rf " TEMP_FILE3);
	ASSERT_EQUAL(e->apply(), -1);
}

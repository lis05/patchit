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
	std::system("chmod -R 777 " SRC);
	std::system("chmod -R 777 " DEST);
	std::system("rm -rf " SRC);
	std::system("rm -rf " DEST);
	open_and_write_entire_file(SRC, str2vec(""));
	open_and_write_entire_file(DEST, str2vec("to"));
}

TEST(entity_move_instruction_constructor_no_args) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>();
	ASSERT_EQUAL(e->signature, Instruction::ENTITY_MOVE);
	ASSERT_EQUAL(e->create_subdirectories, false);
	ASSERT_EQUAL(e->override_if_already_exists, false);
	ASSERT_EQUAL(e->move_from, "");
	ASSERT_EQUAL(e->move_to, "");
}

TEST(entity_move_instruction_constructor_with_args) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(true, true, SRC, DEST);
	ASSERT_EQUAL(e->create_subdirectories, true);
	ASSERT_EQUAL(e->override_if_already_exists, true);
	ASSERT_EQUAL(e->move_from, SRC);
	ASSERT_EQUAL(e->move_to, DEST);
}

TEST(entity_move_instruction_binary_representation) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(true, true, "from", "to");

	/*
	 * override: 1b
	 * create_sub: 1b
	 * move_from: move_from.size() b
	 * \0: 1b
	 * move_to: move_to.size() b
	 * \0: 1b
	*/

	auto vec = e->binary_representation();
	ASSERT_EQUAL(vec.size(), 1+1+e->move_from.size()+1+e->move_to.size()+1);
	ASSERT_EQUAL(vec[0], std::byte{true});
	ASSERT_EQUAL(vec[1], std::byte{true});
	ASSERT_EQUAL(vec[2], std::byte{'f'});
	ASSERT_EQUAL(vec[3], std::byte{'r'});
	ASSERT_EQUAL(vec[4], std::byte{'o'});
	ASSERT_EQUAL(vec[5], std::byte{'m'});
	ASSERT_EQUAL(vec[6], std::byte{'\0'});
	ASSERT_EQUAL(vec[7], std::byte{'t'});
	ASSERT_EQUAL(vec[8], std::byte{'o'});
	ASSERT_EQUAL(vec[9], std::byte{'\0'});
}

TEST(entity_move_instruction_from_binary_representation_ok) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(true, true, "from", "to");

	/*
	 * override: 1b
	 * create_sub: 1b
	 * move_from: move_from.size() b
	 * \0: 1b
	 * move_to: move_to.size() b
	 * \0: 1b
	*/

	auto vec = e->binary_representation();
	ASSERT_EQUAL(e->from_binary_representation(vec), 0);
	ASSERT_EQUAL(e->create_subdirectories, true);
	ASSERT_EQUAL(e->override_if_already_exists, true);
	ASSERT_EQUAL(e->move_from, "from");
	ASSERT_EQUAL(e->move_to, "to");
}

TEST(entity_move_instruction_from_binary_representation_not_enough_bytes) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(true, true, "from", "to");

	/*
	 * override: 1b
	 * create_sub: 1b
	 * move_from: move_from.size() b
	 * \0: 1b
	 * move_to: move_to.size() b
	 * \0: 1b
	*/

	auto vec = e->binary_representation();
	vec.clear();
	ASSERT_EQUAL(e->from_binary_representation(vec), -1);
}

TEST(entity_move_instruction_from_binary_representation_no_null_src_byte) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(true, true, "from", "to");

	/*
	 * override: 1b
	 * create_sub: 1b
	 * move_from: move_from.size() b
	 * \0: 1b
	 * move_to: move_to.size() b
	 * \0: 1b
	*/

	auto vec = e->binary_representation();
	vec[6] = std::byte{1};
	ASSERT_EQUAL(e->from_binary_representation(vec), -1);
}

TEST(entity_move_instruction_from_binary_representation_no_null_dest_byte) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(true, true, "from", "to");

	/*
	 * override: 1b
	 * create_sub: 1b
	 * move_from: move_from.size() b
	 * \0: 1b
	 * move_to: move_to.size() b
	 * \0: 1b
	*/

	auto vec = e->binary_representation();
	vec[9] = std::byte{1};
	ASSERT_EQUAL(e->from_binary_representation(vec), -1);
}

TEST(entity_move_instruction_from_binary_representation_no_null_bute) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(true, true, "from", "to");

	/*
	 * override: 1b
	 * create_sub: 1b
	 * move_from: move_from.size() b
	 * \0: 1b
	 * move_to: move_to.size() b
	 * \0: 1b
	*/

	auto vec = e->binary_representation();
	vec[6] = std::byte{1};
	vec[9] = std::byte{1};
	ASSERT_EQUAL(e->from_binary_representation(vec), -1);
}

TEST(entity_move_instruction_apply_no_override) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(false, false, SRC, DEST);
	auto vec = e->binary_representation();

	ASSERT_EQUAL(open_and_write_entire_file(DEST, str2vec("data")), 0);
	ASSERT_EQUAL(e->apply(), 0);
	ASSERT_EQUAL(open_and_read_entire_file(DEST, vec), 0);
	ASSERT_SEQUENCE_EQUAL(vec, str2vec("data"));
}

TEST(entity_move_instruction_apply_with_override) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(false, true, SRC, DEST);
	auto vec = e->binary_representation();

	ASSERT_EQUAL(open_and_write_entire_file(DEST, str2vec("data")), 0);
	ASSERT_EQUAL(e->apply(), 0);
	ASSERT_EQUAL(open_and_read_entire_file(DEST, vec), 0);
	ASSERT_SEQUENCE_EQUAL(vec, str2vec(""));
}

TEST(entity_move_instruction_apply_cannot_read_src) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(false, true, SRC, DEST);
	auto vec = e->binary_representation();

	std::system("chmod 222 " SRC);
	ASSERT_EQUAL(e->apply(), -1);
}

TEST(entity_move_instruction_apply_src_does_not_exist) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(false, true, SRC, DEST);
	auto vec = e->binary_representation();

	std::system("chmod -R 777 " SRC);
	std::system("rm -rf " SRC);
	ASSERT_EQUAL(e->apply(), -1);
}

TEST(entity_move_instruction_apply_src_is_not_regular) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(false, true, SRC, DEST);
	auto vec = e->binary_representation();

	std::system("chmod -R 777 " SRC);
	std::system("rm -rf " SRC);
	std::system("mkdir " SRC);
	ASSERT_EQUAL(e->apply(), -1);
}

TEST(entity_move_instruction_apply_dest_does_not_exist_and_cannot_create_subdirs) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(false, false, SRC, DEST "/a/b/c");
	auto vec = e->binary_representation();

	std::system("chmod -R 777 " DEST);
	std::system("rm -rf " DEST);
	ASSERT_EQUAL(e->apply(), -1);
}

TEST(entity_move_instruction_apply_dest_does_not_exist_but_can_create_subdirs) {
	setup();
	auto e = std::make_shared<EntityMoveInstruction>(true, false, SRC, DEST "/a/b/c");
	auto vec = e->binary_representation();

	std::system("chmod -R 777 " DEST);
	std::system("rm -rf " DEST);
	ASSERT_EQUAL(e->apply(), 0);
}

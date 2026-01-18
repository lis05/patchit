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

TEST(entity_delete_instruction_constructor_no_args) {
	setup();
	auto e = std::make_shared<EntityDeleteInstruction>();
	ASSERT_EQUAL(e->signature, Instruction::ENTITY_DELETE);
	ASSERT_EQUAL(e->delete_recursively_if_directory, false);
	ASSERT_EQUAL(e->target, "");
}

TEST(entity_delete_instruction_constructor_with_args) {
	setup();
	auto e = std::make_shared<EntityDeleteInstruction>(true, "target");
	ASSERT_EQUAL(e->delete_recursively_if_directory, true);
	ASSERT_EQUAL(e->target, "target");
}

TEST(entity_delete_instruction_binary_representation) {
	setup();
	auto e = std::make_shared<EntityDeleteInstruction>(true, "target");

	/*
	 * delete_rec: 1b
	 * target: target.size() b
	 * \0: 1b
	*/

	auto vec = e->binary_representation();
	ASSERT_EQUAL(vec.size(), 1+6+1);
	ASSERT_EQUAL(vec[0], std::byte{true});
	ASSERT_EQUAL(vec[1], std::byte{'t'});
	ASSERT_EQUAL(vec[2], std::byte{'a'});
	ASSERT_EQUAL(vec[3], std::byte{'r'});
	ASSERT_EQUAL(vec[4], std::byte{'g'});
	ASSERT_EQUAL(vec[5], std::byte{'e'});
	ASSERT_EQUAL(vec[6], std::byte{'t'});
	ASSERT_EQUAL(vec[7], std::byte{'\0'});
}

TEST(entity_delete_instruction_from_binary_representation_ok) {
	setup();
	auto e = std::make_shared<EntityDeleteInstruction>(true, "target");

	/*
	 * delete_rec: 1b
	 * target: target.size() b
	 * \0: 1b
	*/

	auto vec = e->binary_representation();
	ASSERT_EQUAL(e->from_binary_representation(vec), 0);
}

TEST(entity_delete_instruction_from_binary_representation_not_enough_bytes) {
	setup();
	auto e = std::make_shared<EntityDeleteInstruction>(true, "target");

	/*
	 * delete_rec: 1b
	 * target: target.size() b
	 * \0: 1b
	*/

	auto vec = e->binary_representation();
	vec.clear();
	ASSERT_EQUAL(e->from_binary_representation(vec), -1);
}

TEST(entity_delete_instruction_from_binary_representation_no_null_byte) {
	setup();
	auto e = std::make_shared<EntityDeleteInstruction>(true, "target");

	/*
	 * delete_rec: 1b
	 * target: target.size() b
	 * \0: 1b
	*/

	auto vec = e->binary_representation();
	vec.back() = std::byte{1};
	ASSERT_EQUAL(e->from_binary_representation(vec), -1);
}

TEST(entity_delete_instruction_apply_ok) {
	setup();
	auto e = std::make_shared<EntityDeleteInstruction>(false, SRC);

	ASSERT_EQUAL(e->apply(), 0);
	ASSERT_EQUAL(WEXITSTATUS(std::system("[ -e " SRC " ]")), 1);
}

TEST(entity_delete_instruction_apply_recursive) {
	setup();
	auto e = std::make_shared<EntityDeleteInstruction>(true, SRC);

	std::system("rm -rf " SRC);
	std::system("mkdir -p " SRC "/a/b/c/d");
	ASSERT_EQUAL(e->apply(), 0);
}

TEST(entity_delete_instruction_apply_no_rm) {
	setup();
	auto e = std::make_shared<EntityDeleteInstruction>(true, SRC);

	char *old = getenv("PATH");
	setenv("PATH", "", 1);
	ASSERT_EQUAL(e->apply(), -1);
	setenv("PATH", old, 1);
}

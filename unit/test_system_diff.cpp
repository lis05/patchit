#include <unit_test_framework.hpp>
#include <unit_common.hpp>

#include <vector>
#include <cstring>
#include <string>

#include <diff.hpp>
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

static void setup() {
	std::system("chmod -R 777 " SRC);
	std::system("chmod -R 777 " DEST);
	std::system("rm -rf " SRC);
	std::system("rm -rf " DEST);
	open_and_write_entire_file(SRC, str2vec("from"));
	open_and_write_entire_file(DEST, str2vec("to"));
}

TEST(system_diff_constructor) {
	setup();
	std::shared_ptr<Diff> ptr;

	SystemDiff *diff = new SystemDiff();
	ASSERT_NOT_EQUAL(diff, nullptr);
	ASSERT_EQUAL(diff->signature, Diff::SYSTEM_DIFF);
	ptr.reset(diff);
}

TEST(system_diff_from_files_ok) {
	setup();
	std::shared_ptr<SystemDiff> ptr = dynamic_pointer_cast<SystemDiff>(Diff::from_signature(Diff::SYSTEM_DIFF));

	ASSERT_EQUAL(ptr->from_files(SRC, DEST), 0);
	ASSERT_TRUE(!ptr->data.empty());

	ASSERT_EQUAL(ptr->from_files(SRC, DEST), 0); // data is not empty
	ASSERT_TRUE(!ptr->data.empty());
}

TEST(system_diff_from_files_no_xxd) {
	setup();
	std::shared_ptr<SystemDiff> ptr = dynamic_pointer_cast<SystemDiff>(Diff::from_signature(Diff::SYSTEM_DIFF));

	char *old = getenv("PATH");
	setenv("PATH", "", 1);
	ASSERT_EQUAL(ptr->from_files(SRC, DEST), -1);
	ASSERT_TRUE(ptr->data.empty());
	setenv("PATH", old, 1);
}

TEST(system_diff_binary_representation) {
	setup();
	std::shared_ptr<SystemDiff> ptr = dynamic_pointer_cast<SystemDiff>(Diff::from_signature(Diff::SYSTEM_DIFF));
	ptr->compressor = PlainCompressor::get();

	auto vec = ptr->binary_representation();
	ASSERT_EQUAL(vec.size(), 1);
	ASSERT_EQUAL(vec[0], (std::byte)ptr->compressor->get_id());

	ASSERT_EQUAL(ptr->from_files(SRC, DEST), 0);
	vec = ptr->binary_representation();
	ASSERT_TRUE(vec.size() > 1);
	ASSERT_EQUAL(vec[0], (std::byte)ptr->compressor->get_id());
}

TEST(system_diff_from_binary_representation_plain) {
	setup();
	std::shared_ptr<SystemDiff> ptr = dynamic_pointer_cast<SystemDiff>(Diff::from_signature(Diff::SYSTEM_DIFF));
	ptr->compressor = PlainCompressor::get();

	ASSERT_EQUAL(ptr->from_files(SRC, DEST), 0);
	auto vec = ptr->binary_representation();
	auto data = ptr->data;

	ASSERT_EQUAL(ptr->from_binary_representation(vec), 0);
	ASSERT_SEQUENCE_EQUAL(ptr->data, data);
}

TEST(system_diff_from_binary_representation_zlib) {
	setup();
	std::shared_ptr<SystemDiff> ptr = dynamic_pointer_cast<SystemDiff>(Diff::from_signature(Diff::SYSTEM_DIFF));
	ptr->compressor = ZLibCompressor::get();

	ASSERT_EQUAL(ptr->from_files(SRC, DEST), 0);
	auto vec = ptr->binary_representation();
	auto data = ptr->data;

	ASSERT_EQUAL(ptr->from_binary_representation(vec), 0);
	ASSERT_SEQUENCE_EQUAL(ptr->data, data);
}

TEST(system_diff_from_binary_representation_invalid) {
	setup();
	std::shared_ptr<SystemDiff> ptr = dynamic_pointer_cast<SystemDiff>(Diff::from_signature(Diff::SYSTEM_DIFF));
	ptr->compressor = ZLibCompressor::get();

	ASSERT_EQUAL(ptr->from_files(SRC, DEST), 0);
	auto vec = ptr->binary_representation();	vec[0] = std::byte{150};
	auto data = ptr->data;

	ASSERT_EQUAL(ptr->from_binary_representation(vec), -1);

	vec.clear();
	ASSERT_EQUAL(ptr->from_binary_representation(vec), -1);
}

TEST(system_diff_apply_ok) {
	setup();
	auto ptr = dynamic_pointer_cast<SystemDiff>(Diff::from_signature(Diff::SYSTEM_DIFF));
	ptr->compressor = PlainCompressor::get();
	ASSERT_EQUAL(ptr->from_files(SRC, DEST), 0);
	auto vec = ptr->binary_representation();

	std::system("chmod -R 777 " TEMP_FILE3);
	std::system("rm -rf " TEMP_FILE3);
	std::system("cp " SRC " " TEMP_FILE3);

	ASSERT_EQUAL(ptr->apply(TEMP_FILE3), 0);
}

TEST(system_diff_apply_cannot_read) {
	setup();
	auto ptr = dynamic_pointer_cast<SystemDiff>(Diff::from_signature(Diff::SYSTEM_DIFF));
	ptr->compressor = PlainCompressor::get();
	ASSERT_EQUAL(ptr->from_files(SRC, DEST), 0);
	auto vec = ptr->binary_representation();

	std::system("chmod -R 777 " TEMP_FILE3);
	std::system("rm -rf " TEMP_FILE3);
	std::system("cp " SRC " " TEMP_FILE3);
	std::system("chmod 444 " TEMP_FILE3);

	ASSERT_EQUAL(ptr->apply(TEMP_FILE3), -1);
}

TEST(system_diff_apply_empty_diff) {
	setup();
	auto ptr = dynamic_pointer_cast<SystemDiff>(Diff::from_signature(Diff::SYSTEM_DIFF));
	ptr->compressor = PlainCompressor::get();
	ASSERT_EQUAL(ptr->from_files(SRC, DEST), 0);
	ptr->data.clear();
	auto vec = ptr->binary_representation();

	std::system("chmod -R 777 " TEMP_FILE3);
	std::system("rm -rf " TEMP_FILE3);
	std::system("cp " SRC " " TEMP_FILE3);

	ASSERT_EQUAL(ptr->apply(TEMP_FILE3), 0);
}


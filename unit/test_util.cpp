#include <unit_test_framework.hpp>
#include <unit_common.hpp>

#include <vector>
#include <cstring>
#include <string>

#include <util.hpp>
#include <config.hpp>

#define WHERE "/tmp/patchit_unit_dir_test"
#define DIR1 WHERE "/a"
#define DIR2 DIR1 "/b"
#define DIR3 DIR2 "/c"

static void setup() {
	unlink(TEMP_FILE1);
	std::system("rm -rf " WHERE);
}

TEST(util_shorten_size) {
	setup();
	ASSERT_EQUAL(shorten_size(0), "0 B");
	ASSERT_EQUAL(shorten_size(10), "10 B");
	ASSERT_EQUAL(shorten_size(1000), "1000 B");
	ASSERT_EQUAL(shorten_size(1024), "1.0 KiB");
	ASSERT_EQUAL(shorten_size(5*1024), "5.0 KiB");
	ASSERT_EQUAL(shorten_size(1024*1024), "1.0 MiB");
	ASSERT_EQUAL(shorten_size(5*1024*1024), "5.0 MiB");
	ASSERT_EQUAL(shorten_size(1024*1024*1024), "1.0 GiB");
	ASSERT_EQUAL(shorten_size(5ll*1024*1024*1024), "5.0 GiB");
	ASSERT_EQUAL(shorten_size(1024ll*1024*1024*1024), "1.0 TiB");
	ASSERT_EQUAL(shorten_size(5ll*1024*1024*1024*1024), "5.0 TiB");
	ASSERT_EQUAL(shorten_size(50000ll*1024*1024*1024*1024), "50000.0 TiB");
}

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

TEST(util_read_entire_file) {
	setup();
	ASSERT_EQUAL(open_and_write_entire_file(TEMP_FILE1, str2vec("hello")), 0);

	std::vector<std::byte> vec;
	FILE *fd = std::fopen(TEMP_FILE1, "r");
	ASSERT_EQUAL(read_entire_file(TEMP_FILE1, fd, vec), 0);
	ASSERT_EQUAL(vec2str(vec), "hello");
	std::fclose(fd);

	fd = std::fopen(TEMP_FILE1, "w");
	vec.clear();
	ASSERT_EQUAL(read_entire_file(TEMP_FILE1, fd, vec), -1);
	ASSERT_NOT_EQUAL(vec2str(vec), "hello");
	std::fclose(fd);
}

TEST(util_open_and_read_entire_file) {
	setup();
	ASSERT_EQUAL(open_and_write_entire_file(TEMP_FILE1, str2vec("hello")), 0);

	std::vector<std::byte> vec;
	ASSERT_EQUAL(open_and_read_entire_file(TEMP_FILE1, vec), 0);
	ASSERT_EQUAL(vec2str(vec), "hello");

	std::system("chmod 222 " TEMP_FILE1);
	ASSERT_EQUAL(open_and_read_entire_file(TEMP_FILE1, vec), -1);

	unlink(TEMP_FILE1);
	ASSERT_EQUAL(open_and_read_entire_file(TEMP_FILE1, vec), -1);
}

TEST(util_write_entire_file) {
	setup();

	std::vector<std::byte> vec;
	FILE *fd = std::fopen(TEMP_FILE1, "w");
	ASSERT_EQUAL(write_entire_file(TEMP_FILE1, fd, str2vec("hello")), 0);
	std::fclose(fd);
	ASSERT_EQUAL(open_and_read_entire_file(TEMP_FILE1, vec), 0);
	ASSERT_EQUAL(vec2str(vec), "hello");

	fd = std::fopen(TEMP_FILE1, "r");
	ASSERT_EQUAL(write_entire_file(TEMP_FILE1, fd, str2vec("hello")), -1);
	std::fclose(fd);
}

TEST(util_open_and_write_entire_file) {
	setup();

	std::vector<std::byte> vec;
	ASSERT_EQUAL(open_and_write_entire_file(TEMP_FILE1, str2vec("hello")), 0);
	ASSERT_EQUAL(open_and_read_entire_file(TEMP_FILE1, vec), 0);
	ASSERT_EQUAL(vec2str(vec), "hello");

	ASSERT_EQUAL(open_and_write_entire_file(TEMP_FILE1, str2vec("overwrite")), 0);
	ASSERT_EQUAL(open_and_read_entire_file(TEMP_FILE1, vec), 0);
	ASSERT_EQUAL(vec2str(vec), "overwrite");

	std::system("chmod 444 " TEMP_FILE1);
	ASSERT_EQUAL(open_and_write_entire_file(TEMP_FILE1, str2vec("hello")), -1);
}

TEST(util_store_uint64_t_restore_uint64_t) {
	setup();

	std::vector<std::byte> vec;
	uint64_t val;
	std::vector<std::byte>::iterator it;

	store_uint64_t(175, vec);
	it = vec.begin();
	ASSERT_EQUAL(restore_uint64_t(it, vec.end(), val), 0);
	ASSERT_EQUAL(val, 175);

	store_uint64_t(999, vec);
	it = vec.begin() + 8;
	ASSERT_EQUAL(restore_uint64_t(it, vec.end(), val), 0);
	ASSERT_EQUAL(val, 999);

	it = vec.end() - 7;
	ASSERT_EQUAL(restore_uint64_t(it, vec.end(), val), -1);
}

TEST(util_mkdirr) {
	setup();
	char *f;

	f = strdup(DIR3);
	mkdirr(f, 0777);
	ASSERT_EQUAL(WEXITSTATUS(std::system("[ -d " DIR3 " ]")), 0);

	std::system("rm -rf " WHERE);
}

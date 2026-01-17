#include <unit_test_framework.hpp>
#include <unit_common.hpp>

#include <vector>
#include <cstring>
#include <string>

#include <compressor.hpp>
#include <config.hpp>

static void setup() {
}

TEST(plain_compressor_get) {
	ASSERT_NOT_EQUAL(PlainCompressor::get(), nullptr);
	PlainCompressor::get()->get_id();
}

TEST(plain_compressor_compress_decompress) {
	std::vector<std::byte> vec{std::byte{1}, std::byte{2}, std::byte{3}};

	ASSERT_SEQUENCE_EQUAL(vec, PlainCompressor::get()->compress(vec));
	ASSERT_SEQUENCE_EQUAL(vec, PlainCompressor::get()->decompress(vec));

	vec.clear();

	ASSERT_SEQUENCE_EQUAL(vec, PlainCompressor::get()->compress(vec));
	ASSERT_SEQUENCE_EQUAL(vec, PlainCompressor::get()->decompress(vec));
}

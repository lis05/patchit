#include <unit_test_framework.hpp>
#include <unit_common.hpp>

#include <vector>
#include <cstring>
#include <string>

#include <compressor.hpp>
#include <config.hpp>

static void setup() {
}

TEST(zlib_compressor_get) {
	ASSERT_NOT_EQUAL(ZLibCompressor::get(), nullptr);
	ZLibCompressor::get()->get_id();
}

TEST(zlib_compressor_compress_decompress) {
	std::vector<std::byte> data{std::byte{1}, std::byte{2}, std::byte{3}};
	std::vector<std::byte> res;

	res = ZLibCompressor::get()->compress(data);
	ASSERT_TRUE(res.size() > 0);
	ASSERT_SEQUENCE_EQUAL(data, ZLibCompressor::get()->decompress(res));

	// replace original size
	for (int i = 0; i < 8; i++) res[i] = std::byte{0};
	ASSERT_TRUE(ZLibCompressor::get()->decompress(res).empty());

	res.clear();
	ASSERT_TRUE(ZLibCompressor::get()->decompress(res).empty());
}

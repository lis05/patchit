#include <config.hpp>
#include <cstring>
#include <patch.hpp>
#include <string>
#include <unit_common.hpp>
#include <unit_test_framework.hpp>
#include <util.hpp>
#include <vector>

TEST(instruction_set_compressor) {
	auto i = std::make_shared<EntityMoveInstruction>();

	i->set_compressor(PlainCompressor::get());
	ASSERT_EQUAL(i->compressor, PlainCompressor::get());
}

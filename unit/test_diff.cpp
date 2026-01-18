#include <unit_test_framework.hpp>
#include <unit_common.hpp>

#include <vector>
#include <cstring>
#include <string>

#include <diff.hpp>
#include <config.hpp>

static void setup() {
}

TEST(diff_from_signature) {
	auto ptr = Diff::from_signature(Diff::SYSTEM_DIFF);
	SystemDiff *p;
	ASSERT_NOT_EQUAL(p = dynamic_cast<SystemDiff*>(ptr.get()), nullptr);
	ASSERT_EQUAL(p->signature, Diff::SYSTEM_DIFF);

	ASSERT_EQUAL(Diff::from_signature(150), nullptr);
}

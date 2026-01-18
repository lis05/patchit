#include <config.hpp>
#include <cstring>
#include <patch.hpp>
#include <string>
#include <unit_common.hpp>
#include <unit_test_framework.hpp>
#include <util.hpp>
#include <vector>

static std::vector<std::byte> str2vec(std::string s) {
    std::vector<std::byte> res;
    for (auto c : s) res.push_back((std::byte)c);
    return res;
}

static std::string vec2str(std::vector<std::byte> v) {
    std::string res;
    for (auto b : v) res += (char)b;
    return res;
}

#define SRC TEMP_FILE1
#define DEST TEMP_FILE2
#define PATCH TEMP_FILE3

static void setup() {
    std::system("chmod -R 777 " SRC);
    std::system("chmod -R 777 " DEST);
    std::system("chmod -R 777 " PATCH);
    std::system("rm -rf " SRC);
    std::system("rm -rf " DEST);
    std::system("rm -rf " PATCH);
    open_and_write_entire_file(SRC, str2vec(""));
    open_and_write_entire_file(DEST, str2vec("to"));
}

TEST(patch_instruction_from_signature) {
    setup();
    ASSERT_NOT_EQUAL(
        dynamic_cast<EntityModifyInstruction*>(
            Instruction::from_signature(Instruction::ENTITY_MODIFY).get()),
        nullptr);

    ASSERT_NOT_EQUAL(
        dynamic_cast<EntityMoveInstruction*>(
            Instruction::from_signature(Instruction::ENTITY_MOVE).get()),
        nullptr);

    ASSERT_NOT_EQUAL(
        dynamic_cast<EntityDeleteInstruction*>(
            Instruction::from_signature(Instruction::ENTITY_DELETE).get()),
        nullptr);

    ASSERT_EQUAL(Instruction::from_signature(150), nullptr);
}

TEST(patch_append) {
    setup();
    auto p = std::make_shared<Patch>();

    auto i1 = std::make_shared<EntityModifyInstruction>();
    p->append(i1);
    ASSERT_EQUAL(p->instructions.size(), 1);
    ASSERT_EQUAL(p->instructions.back(), i1);

    auto i2 = std::make_shared<EntityMoveInstruction>();
    p->append(i2);
    ASSERT_EQUAL(p->instructions.size(), 2);
    ASSERT_EQUAL(p->instructions.back(), i2);

    auto i3 = std::make_shared<EntityDeleteInstruction>();
    p->append(i3);
    ASSERT_EQUAL(p->instructions.size(), 3);
    ASSERT_EQUAL(p->instructions.back(), i3);
}

TEST(patch_inspect_contents) {
    setup();
    // cant really test, so lets just increase coverage xD
    auto p = std::make_shared<Patch>();

    auto i1 = std::make_shared<EntityModifyInstruction>();
    p->append(i1);

    auto i2 = std::make_shared<EntityMoveInstruction>();
    p->append(i2);

    auto i3 = std::make_shared<EntityDeleteInstruction>();
    p->append(i3);

    p->inspect_contents(0);
    p->inspect_contents(1);
    p->inspect_contents(2);
    p->inspect_contents(3);
}

TEST(patch_write_to_file) {
    setup();
    auto p = std::make_shared<Patch>();
    auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
    d->compressor = PlainCompressor::get();
    ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
    auto i1 = std::make_shared<EntityModifyInstruction>(false, false, SRC, d);
    i1->compressor = PlainCompressor::get();
    p->append(i1);

    ASSERT_EQUAL(p->write_to_file(PATCH), 0);
    auto p2 = std::make_shared<Patch>();
    ASSERT_EQUAL(p2->load_from_file(PATCH), 0);
    ASSERT_EQUAL(p2->instructions.size(), 1);
    ASSERT_EQUAL(p2->instructions.back()->signature, Instruction::ENTITY_MODIFY);
}

TEST(patch_write_to_file_then_load_into_the_same_object) {
    setup();
    auto p = std::make_shared<Patch>();
    auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());
    d->compressor = PlainCompressor::get();
    ASSERT_EQUAL(d->from_files(SRC, DEST), 0);
    auto i1 = std::make_shared<EntityModifyInstruction>(false, false, SRC, d);
    i1->compressor = PlainCompressor::get();
    p->append(i1);

    ASSERT_EQUAL(p->write_to_file(PATCH), 0);
    ASSERT_EQUAL(p->load_from_file(PATCH), 0);
    ASSERT_EQUAL(p->instructions.size(), 1);
    ASSERT_EQUAL(p->instructions.back()->signature, Instruction::ENTITY_MODIFY);
}

TEST(patch_load_from_file_missing_file) {
    setup();
    auto p = std::make_shared<Patch>();

    ASSERT_EQUAL(p->load_from_file(PATCH), -1);
}

#define setup_simple_patchfile                                                 \
    auto p = std::make_shared<Patch>();                                        \
	auto p2 = std::make_shared<Patch>(); \
    auto d = static_pointer_cast<Diff>(std::make_shared<SystemDiff>());        \
    d->compressor = PlainCompressor::get();                                    \
    ASSERT_EQUAL(d->from_files(SRC, DEST), 0);                                 \
    auto i1 = std::make_shared<EntityModifyInstruction>(false, false, SRC, d); \
    i1->compressor = PlainCompressor::get();                                   \
    p->append(i1);                                                             \
    ASSERT_EQUAL(p->write_to_file(PATCH), 0);                                  \
                                                                               \
    std::vector<std::byte> data;                                               \
    ASSERT_EQUAL(open_and_read_entire_file(PATCH, data), 0);

TEST(patch_load_from_file_invalid_patch_signature) {
    setup();
    setup_simple_patchfile;

    /*
     * Binary representation:
     *
     * SIGNATURE(with NULL byte)
     * compatibility_version (uint64, least to most significant)
     * number_of_instructions (uint64_t, least to most significant)
     * len_I1 (uint64_t, ...)
     * I1_signature (1byte)
     * I1
     * len_I2 (uint64_t, ...)
     * I2_signature (1byte)
     * I2
     * ...
     */
    data[0] = std::byte{0};

    ASSERT_EQUAL(open_and_write_entire_file(PATCH, data), 0);
    ASSERT_EQUAL(p2->load_from_file(PATCH), -1);
}

TEST(patch_load_from_file_missing_signature_null_byte) {
    setup();
    setup_simple_patchfile;

    /*
     * Binary representation:
     *
     * SIGNATURE(with NULL byte)
     * compatibility_version (uint64, least to most significant)
     * number_of_instructions (uint64_t, least to most significant)
     * len_I1 (uint64_t, ...)
     * I1_signature (1byte)
     * I1
     * len_I2 (uint64_t, ...)
     * I2_signature (1byte)
     * I2
     * ...
     */
    data[strlen("__PATCHIT__")] = std::byte{1};

    ASSERT_EQUAL(open_and_write_entire_file(PATCH, data), 0);
    ASSERT_EQUAL(p2->load_from_file(PATCH), -1);
}

TEST(patch_load_from_file_not_enough_bytes_for_compatibility_version) {
    setup();
    setup_simple_patchfile;

    /*
     * Binary representation:
     *
     * SIGNATURE(with NULL byte)
     * compatibility_version (uint64, least to most significant)
     * number_of_instructions (uint64_t, least to most significant)
     * len_I1 (uint64_t, ...)
     * I1_signature (1byte)
     * I1
     * len_I2 (uint64_t, ...)
     * I2_signature (1byte)
     * I2
     * ...
     */
    data.erase(data.begin() + strlen("__PATCHIT__") + 1, data.end());

    ASSERT_EQUAL(open_and_write_entire_file(PATCH, data), 0);
    ASSERT_EQUAL(p2->load_from_file(PATCH), -1);
}

TEST(patch_load_from_file_incompatible_version) {
    setup();
    setup_simple_patchfile;

    /*
     * Binary representation:
     *
     * SIGNATURE(with NULL byte)
     * compatibility_version (uint64, least to most significant)
     * number_of_instructions (uint64_t, least to most significant)
     * len_I1 (uint64_t, ...)
     * I1_signature (1byte)
     * I1
     * len_I2 (uint64_t, ...)
     * I2_signature (1byte)
     * I2
     * ...
     */
	int pos = strlen("__PATCHIT__") + 1;
	for (int i = 0; i < 8; i++) data[pos + i] = std::byte{1};

    ASSERT_EQUAL(open_and_write_entire_file(PATCH, data), 0);
    ASSERT_EQUAL(p2->load_from_file(PATCH), -1);
}

TEST(patch_load_from_file_too_few_bytes_for_number_of_instructions) {
    setup();
    setup_simple_patchfile;

    /*
     * Binary representation:
     *
     * SIGNATURE(with NULL byte)
     * compatibility_version (uint64, least to most significant)
     * number_of_instructions (uint64_t, least to most significant)
     * len_I1 (uint64_t, ...)
     * I1_signature (1byte)
     * I1
     * len_I2 (uint64_t, ...)
     * I2_signature (1byte)
     * I2
     * ...
     */
	int pos = strlen("__PATCHIT__") + 1;
	data.erase(data.begin() + pos + 8, data.end());

    ASSERT_EQUAL(open_and_write_entire_file(PATCH, data), 0);
    ASSERT_EQUAL(p2->load_from_file(PATCH), -1);
}

TEST(patch_load_from_file_too_few_bytes_for_first_instruction_len) {
    setup();
    setup_simple_patchfile;

    /*
     * Binary representation:
     *
     * SIGNATURE(with NULL byte)
     * compatibility_version (uint64, least to most significant)
     * number_of_instructions (uint64_t, least to most significant)
     * len_I1 (uint64_t, ...)
     * I1_signature (1byte)
     * I1
     * len_I2 (uint64_t, ...)
     * I2_signature (1byte)
     * I2
     * ...
     */
	int pos = strlen("__PATCHIT__") + 1;
	data.erase(data.begin() + pos + 8 + 8, data.end());

    ASSERT_EQUAL(open_and_write_entire_file(PATCH, data), 0);
    ASSERT_EQUAL(p2->load_from_file(PATCH), -1);
}

TEST(patch_load_from_file_corrupted_instruction_signature) {
    setup();
    setup_simple_patchfile;

    /*
     * Binary representation:
     *
     * SIGNATURE(with NULL byte)
     * compatibility_version (uint64, least to most significant)
     * number_of_instructions (uint64_t, least to most significant)
     * len_I1 (uint64_t, ...)
     * I1_signature (1byte)
     * I1
     * len_I2 (uint64_t, ...)
     * I2_signature (1byte)
     * I2
     * ...
     */
	int pos = strlen("__PATCHIT__") + 1;
	data[pos + 8 + 8 + 8] = std::byte{150};

    ASSERT_EQUAL(open_and_write_entire_file(PATCH, data), 0);
    ASSERT_EQUAL(p2->load_from_file(PATCH), -1);
}

TEST(patch_load_from_file_truncated_instruction) {
    setup();
    setup_simple_patchfile;

    /*
     * Binary representation:
     *
     * SIGNATURE(with NULL byte)
     * compatibility_version (uint64, least to most significant)
     * number_of_instructions (uint64_t, least to most significant)
     * len_I1 (uint64_t, ...)
     * I1_signature (1byte)
     * I1
     * len_I2 (uint64_t, ...)
     * I2_signature (1byte)
     * I2
     * ...
     */
	int pos = strlen("__PATCHIT__") + 1;
	data.erase(data.begin()+pos+8+8+8+1, data.end());

    ASSERT_EQUAL(open_and_write_entire_file(PATCH, data), 0);
    ASSERT_EQUAL(p2->load_from_file(PATCH), -1);
}

TEST(patch_load_from_file_corrupted_instruction) {
    setup();
    setup_simple_patchfile;

    /*
     * Binary representation:
     *
     * SIGNATURE(with NULL byte)
     * compatibility_version (uint64, least to most significant)
     * number_of_instructions (uint64_t, least to most significant)
     * len_I1 (uint64_t, ...)
     * I1_signature (1byte)
     * I1
     * len_I2 (uint64_t, ...)
     * I2_signature (1byte)
     * I2
     * ...
     */
	int pos = strlen("__PATCHIT__") + 1;
	data[pos+8+8+8+1] = std::byte{150};

    ASSERT_EQUAL(open_and_write_entire_file(PATCH, data), 0);
    ASSERT_EQUAL(p2->load_from_file(PATCH), -1);
}

TEST(patch_apply_ok) {
    setup();
    setup_simple_patchfile;

	ASSERT_EQUAL(p->apply(), 0);
}

TEST(patch_apply_fail) {
    setup();
    setup_simple_patchfile;

	std::system("rm -rf " SRC);
	ASSERT_EQUAL(p->apply(), -1);
}

#pragma once
#include <compressor.hpp>
#include <cstddef>
#include <diff.hpp>
#include <memory>
#include <string>
#include <vector>

class Instruction {
protected:
    friend class Patch;

    std::shared_ptr<Compressor> compressor;

public:
    enum InstructionSignature : uint8_t {
        ENTITY_MOVE,
        ENTITY_DELETE,
        ENTITY_MODIFY,
        ENTITY_CHANGE_PERMISSIONS
    } signature;
    /*
     * Apply this instruction. Returns 0 on success.
     */
    virtual int apply() = 0;

    /*
     * Return uncompressed binary representation of the instruction.
     */
    virtual std::vector<std::byte> binary_representation() = 0;

    /*
     * Reconstruct the instruction from its given binary representation.
     * Returns 0 on success.
     */
    virtual int from_binary_representation(const std::vector<std::byte> &data) = 0;

    /*
     * Select the desired Compressor to use.
     */
    void set_compressor(std::shared_ptr<Compressor> compressor);

    static std::shared_ptr<Instruction> from_signature(uint8_t signature);
};

class EntityMoveInstruction : public Instruction {
private:
    friend class Patch;

    bool create_empty_file_if_not_exists;
    bool create_empty_directory_if_not_exists;
    bool override_if_already_exists;

    std::string move_from;
    std::string move_to;

public:
    EntityMoveInstruction(bool               create_empty_file_if_not_exists,
                          bool               create_empty_directory_if_not_exists,
                          bool               override_if_already_exists,
                          const std::string &move_from, const std::string &move_to);
    EntityMoveInstruction() = default;

    int                    apply() override;
    std::vector<std::byte> binary_representation() override;
    int from_binary_representation(const std::vector<std::byte> &data) override;
};

class EntityDeleteInstruction : public Instruction {
private:
    friend class Patch;

    bool delete_recursively_if_directory;

    std::string target;

public:
    EntityDeleteInstruction(bool        delete_recursively_if_directory,
                            std::string target);
    EntityDeleteInstruction() = default;

    int                    apply() override;
    std::vector<std::byte> binary_representation() override;
    int from_binary_representation(const std::vector<std::byte> &data) override;
};

class EntityModifyInstruction : public Instruction {
private:
    friend class Patch;

    bool create_subdirectories;
    bool create_empty_file_if_not_exists;

    std::string           target;
    std::shared_ptr<Diff> diff;

public:
    EntityModifyInstruction(bool create_subdirectories,
                            bool create_empty_file_if_not_exists, std::string target,
                            std::shared_ptr<Diff> diff);
    EntityModifyInstruction();

    int                    apply() override;
    std::vector<std::byte> binary_representation() override;
    int from_binary_representation(const std::vector<std::byte> &data) override;
};

class EntityChangePermissionsInstruction : public Instruction {
private:
    friend class Patch;

    bool create_empty_file_if_not_exists;
    bool create_empty_directory_if_not_exists;
    bool apply_recursively_if_directory;

    uint64_t flags;  // 4=r, 2=w, 1=x

public:
    EntityChangePermissionsInstruction(bool     create_empty_file_if_not_exists,
                                       bool     create_empty_directory_if_not_exists,
                                       bool     apply_recursively_if_directory,
                                       uint64_t flags);
    EntityChangePermissionsInstruction() = default;

    int                    apply() override;
    std::vector<std::byte> binary_representation() override;
    int from_binary_representation(const std::vector<std::byte> &data) override;
};

class Patch {
private:
    /*
     * Compatibility version of the source code for which this patch was created.
     * Used to check compatibility.
     */
    static const uint64_t compatibility_version = 0;

    std::vector<std::shared_ptr<Instruction>> instructions;

public:
    /*
     * Apply this patch. Returns 0 on success.
     */
    int apply();

    /*
     * Append the given instruction to the end of the instructions list.
     */
    void append(std::shared_ptr<Instruction> instruction);

    int write_to_file(const std::string &file);
    int load_from_file(const std::string &file);

    void inspect_contents(int verbosity);
};

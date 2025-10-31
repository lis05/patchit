#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class Compressor;

class Instruction {
protected:
    std::shared_ptr<Compressor> compressor;

public:
    /*
     * Apply this instruction. Returns 0 on success.
     */
    virtual int apply() = 0;

    /*
     * Return uncompressed binary reprezentation of the instruction.
     */
    virtual std::vector<std::byte> binary_reprezentation() = 0;

    /*
     * Reconstruct the instruction from its given binary reprezentation.
     * Returns 0 on success.
     */
    virtual int from_binary_reprezentation(const std::vector<std::byte> &data);

    /*
     * Select the desired Compressor to use.
     */
    void set_compressor(std::shared_ptr<Compressor> compressor);
};

class EntityMoveInstruction : public Instruction {
private:
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

    int                    apply() override;
    std::vector<std::byte> binary_reprezentation() override;
    int from_binary_reprezentation(const std::vector<std::byte> &data) override;
};

class EntityDeleteInstruction : public Instruction {
private:
    bool delete_recursively_if_directory;

    std::string target;

public:
    EntityDeleteInstruction(bool        delete_recursively_if_directory,
                            std::string target);

    int                    apply() override;
    std::vector<std::byte> binary_reprezentation() override;
    int from_binary_reprezentation(const std::vector<std::byte> &data) override;
};

class Diff;

class EntityModifyInstruction : public Instruction {
private:
    bool is_whole_file_included;
    bool recreate_target_if_not_exists;
    bool create_empty_file_if_not_exists;

    std::unique_ptr<Diff> diff;

public:
    EntityModifyInstruction(bool                  is_whole_file_included,
                            bool                  recreate_target_if_not_exists,
                            bool                  create_empty_file_if_not_exists,
                            std::unique_ptr<Diff> diff);

    int                    apply() override;
    std::vector<std::byte> binary_reprezentation() override;
    int from_binary_reprezentation(const std::vector<std::byte> &data) override;
};

class EntityChangePermissionsInstruction : public Instruction {
private:
    bool create_empty_file_if_not_exists;
    bool create_empty_directory_if_not_exists;
    bool apply_recursively_if_directory;

    uint64_t flags;  // 4=r, 2=w, 1=x

public:
    EntityChangePermissionsInstruction(bool     create_empty_file_if_not_exists,
                                       bool     create_empty_directory_if_not_exists,
                                       bool     apply_recursively_if_directory,
                                       uint64_t flags);

    int                    apply() override;
    std::vector<std::byte> binary_reprezentation() override;
    int from_binary_reprezentation(const std::vector<std::byte> &data) override;
};

class Patch {
private:
    /*
     * Compatibility version of the source code for which this patch was created.
     * Used to check compatibility.
     */
    uint64_t compatibility_version;

    std::vector<std::unique_ptr<Instruction>> instructions;

public:
    /*
     * Create an empty patch.
     */
    Patch(uint64_t compatibility_version);

    /*
     * Reconstruct a patch based on the provided file.
     */
    Patch(const std::string &filepath);

    /*
     * Apply this patch. Returns 0 on success.
     */
    int apply();

    /*
     * Append the given instruction to the end of the instructions list.
     */
    void append(std::unique_ptr<Instruction> instruction);
};

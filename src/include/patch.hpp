#pragma once
#include <cstddef>
#include <string>
#include <vector>

class Instruction {
public:
    /*
     * Version of the source code for which this instruction was created.
     * Used to check compatibility.
     */
    uint64_t version;

    /*
     * Check whether this instuction is compatible with the latest
     * code and whether it can be applied.
     */
    virtual bool check_compatibility() = 0;

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
};

class EntityMoveInstruction : public Instruction {};

class EntityDeleteInstruction : public Instruction {};

class EntityModifyInstruction : public Instruction {};

class EntityChangePermissionsInstruction : public Instruction {};

class Patch {
private:
public:
};

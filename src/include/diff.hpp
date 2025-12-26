#pragma once
#include <compressor.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <variant>
#include <vector>

class Diff {
public:
    enum DiffSignature : uint8_t {
        SYSTEM_DIFF,
    } signature;

    std::shared_ptr<Compressor> compressor;
    /*
     * Construct the diff from the given files.
     * */
    virtual int from_files(const std::string &src, const std::string &dest) = 0;

    /*
     * Return uncompressed binary representation of the diff.
     */
    virtual std::vector<std::byte> binary_representation() = 0;

    /*
     * Reconstruct the diff from its given binary representation.
     * Returns 0 on success.
     */
    virtual int from_binary_representation(const std::vector<std::byte> &data) = 0;

    /*
     * Apply this patch to the given file. Returns 0 on success.
     */
    virtual int apply(const std::string &file) = 0;

    static std::shared_ptr<Diff> from_signature(uint8_t signature);
};

/*
 * Uses diff, od, and patch.
 */
class SystemDiff : public Diff {
private:
    std::vector<std::byte> data;

public:
    SystemDiff();
    int from_files(const std::string &src, const std::string &dest) override;
    std::vector<std::byte> binary_representation() override;
    int from_binary_representation(const std::vector<std::byte> &data) override;
    int apply(const std::string &file) override;
};

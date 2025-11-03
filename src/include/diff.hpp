#pragma once
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

class Diff {
public:
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
     * Apply this patch to the data. Returns empty vector on error.
     */
    virtual std::vector<std::byte> apply(const std::vector<std::byte> &data) = 0;
};


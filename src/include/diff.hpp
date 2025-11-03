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

class NativeDiff : public Diff {
private:
    enum RecordType : uint8_t {
        ADD,
        DELETE,
    };

    class Record {
    public:
        RecordType type;
        uint64_t   pos; /* Location in the source data. */

        /* If type is DELETE, data will be empty, and len will contain the length. */
        std::variant<std::vector<std::byte>, size_t> data;

        Record(RecordType type, uint64_t pos);
    };

private:
    std::vector<Record> changes;

    /*
     * Populate ::changes. Returns 0 on success.
     */
    int calculate(size_t src_size, std::byte *src_data, size_t dest_size,
                  std::byte *dest_data);

public:
    NativeDiff() = default;

    int from_files(const std::string &src, const std::string &dest) override;
    std::vector<std::byte> binary_representation() override;
    int from_binary_representation(const std::vector<std::byte> &data) override;
    std::vector<std::byte> apply(const std::vector<std::byte> &data);
};

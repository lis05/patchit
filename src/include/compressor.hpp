#pragma once

#include <cstddef>
#include <memory>
#include <vector>

class Compressor {
public:
    /*
     * Compressor ID.
     */
    virtual int getId() = 0;

    /*
     * Compress the provided data.
     */
    virtual std::vector<std::byte> compress(const std::vector<std::byte> &data) = 0;

    /*
     * Decompress the provided data. Returns an empty vector on error.
     */
    virtual std::vector<std::byte> decompress(
        const std::vector<std::byte> &data) = 0;
};

/*
 * Doesn't compress the data, returning the original bytes instead.
 */
class PlainCompressor : public Compressor {
private:
    PlainCompressor();

public:
    static std::shared_ptr<PlainCompressor> get();

    int                    getId() override;
    std::vector<std::byte> compress(const std::vector<std::byte> &data) override;
    std::vector<std::byte> decompress(const std::vector<std::byte> &data) override;
};

/*
 * Uses zlib to compress the data.
 */
class ZLibCompressor : public Compressor {
private:
    ZLibCompressor();

public:
    static std::shared_ptr<ZLibCompressor> get();

    int                    getId() override;
    std::vector<std::byte> compress(const std::vector<std::byte> &data) override;
    std::vector<std::byte> decompress(const std::vector<std::byte> &data) override;
};

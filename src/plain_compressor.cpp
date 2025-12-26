#include <compressor.hpp>

PlainCompressor::PlainCompressor() {
}

std::shared_ptr<PlainCompressor> PlainCompressor::get() {
    static std::shared_ptr<PlainCompressor> instance;
    if (!instance) {
        instance.reset(new PlainCompressor());
    }
    return instance;
}

int PlainCompressor::getId() {
    return 0;
}

std::vector<std::byte> PlainCompressor::compress(
    const std::vector<std::byte> &data) {
    return data;
}

std::vector<std::byte> PlainCompressor::decompress(
    const std::vector<std::byte> &data) {
    return data;
}

#include <compressor.hpp>

std::vector<std::byte> PlainCompressor::compress(
    const std::vector<std::byte> &data) {
    return data;
}

std::vector<std::byte> PlainCompressor::decompress(
    const std::vector<std::byte> &data) {
    return data;
}

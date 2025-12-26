#include <zlib.h>

#include <compressor.hpp>
#include <error.hpp>
#include <util.hpp>

ZLibCompressor::ZLibCompressor() {
}

std::shared_ptr<ZLibCompressor> ZLibCompressor::get() {
    static std::shared_ptr<ZLibCompressor> instance;
    if (!instance) {
        instance.reset(new ZLibCompressor());
    }
    return instance;
}

int ZLibCompressor::getId() {
    return 1;
}

std::vector<std::byte> ZLibCompressor::compress(const std::vector<std::byte> &data) {
    std::vector<std::byte> ret;

    try {
        ret.resize(compressBound(data.size()));
    } catch (...) {
        ERROR("Out of memory.\n");
        return {};
    }

    uLong comprLen = ret.size();
    int   err = ::compress((Byte *)ret.data(), &comprLen, (const Bytef *)data.data(),
                           data.size());

    if (err == Z_MEM_ERROR) {
        ERROR("Zlib failed: out of memory\n");
        return {};
    } else if (err == Z_BUF_ERROR) {
        ERROR("Zlib failed: compresBound gave incorrect estimate\n");
        return {};
    }

    ret.resize(comprLen);

    /* Insert original size. */
    store_uint64_t((uint64_t)data.size(), ret);

    MSG("ZLib compressed: %s -> %s\n", shorten_size(data.size()).c_str(),
        shorten_size(ret.size()).c_str());

    return ret;
}

std::vector<std::byte> ZLibCompressor::decompress(
    const std::vector<std::byte> &data) {
    uint64_t               dest_size;
    std::vector<std::byte> ret;

    if (data.size() < 8) {
        ERROR("Corrupted data: less than 8 bytes\n");
        return {};
    }

    /* I suck at C++ */
    for (int i = 0; i < 8; i++) {
        ret.push_back(*(data.end() - 8 + i));
    }
    std::vector<std::byte>::iterator it = ret.end() - 8;
    restore_uint64_t(it, ret.end(), dest_size);
    ret.clear();

    try {
        ret.resize(dest_size);
    } catch (...) {
        ERROR("Out of memory.\n");
        return {};
    }

    uLongf sz = dest_size;
    int    err =
        ::uncompress((Byte *)ret.data(), &sz, (Byte *)data.data(), data.size());

    if (err == Z_MEM_ERROR) {
        ERROR("Zlib failed: out of memory\n");
        return {};
    } else if (err == Z_BUF_ERROR) {
        ERROR("Zlib failed: corrupted data: original size is wrong\n");
        return {};
    } else if (err == Z_DATA_ERROR) {
        ERROR("Zlib failed: corrupted data\n");
        return {};
    }

    ret.resize(sz);

    return ret;
}

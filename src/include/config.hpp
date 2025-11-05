#pragma once

#include <memory>
#include <string>

class Compressor;

class Config {
public:
    std::string version;
    uint64_t    compatibility_version;

    int verbosity;

    std::shared_ptr<Compressor> compressor;

private:
    Config();

public:
    static std::shared_ptr<Config> get();
};

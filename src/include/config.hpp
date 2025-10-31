#pragma once

#include <string>

class Config {
public:
    std::string version;
    uint64_t    compatibility_version;

    int verbosity;

private:
    Config();

public:
    static Config& get();
};

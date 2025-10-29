#pragma once

class Config {
public:
    int verbosity;

private:
    Config();

public:
    static Config& get();
};

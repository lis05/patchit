#include <config.hpp>

Config::Config() {
    this->verbosity = 0;
}

Config &Config::get() {
    static Config *instance = nullptr;
    if (!instance)
        instance = new Config();
    return *instance;
}

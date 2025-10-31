#include <config.hpp>

Config::Config() {
    this->verbosity = 0;

#ifdef PATCHIT_VERSION
    this->version = PATCHIT_VERSION;
#else
    this->version = "unknown";
#endif

#ifdef PATCHIT_COMPATIBILITY_VERSION
    this->compatibility_version = PATCHIT_COMPATIBILITY_VERSION;
    this->version += "." + std::to_string(this->compatibility_version);
#else
    this->compatibility_version = (uint64_t)-1;  // incompatible
    this->version += ".incompatible";
#endif
}

Config &Config::get() {
    static Config *instance = nullptr;
    if (!instance) {
        instance = new Config();
    }
    return *instance;
}

#include <diff.hpp>
#include <error.hpp>

std::shared_ptr<Diff> Diff::from_signature(uint8_t signature) {
    std::shared_ptr<Diff> res;
    switch (signature) {
    case Diff::SYSTEM_DIFF:
        INFO("Diff signature recognized: SystemDiff\n");
        res.reset(new SystemDiff());
        break;
    }
    if (!res) {
        WARN("Unrecognized diff signature: %d\n", (int)signature);
    }
    return res;
}

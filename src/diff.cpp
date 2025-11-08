#include <diff.hpp>

std::shared_ptr<Diff> Diff::from_signature(uint8_t signature) {
    std::shared_ptr<Diff> res;
    switch (signature) {
    case Diff::SYSTEM_DIFF:
        res.reset(new SystemDiff());
		break;
    }
    return res;
}

#include <sstream>
#include <util.hpp>

std::string shorten_size(size_t bytes) {
    std::string units[] = {"KiB", "MiB", "GiB", "TiB"};
    std::string selected = "B";
    double      count = bytes;
    for (auto unit : units) {
        if (bytes >= 1024) {
            count /= 1024;
            bytes /= 1024;
            selected = unit;
        }
    }

	if (selected == "B") {
		return std::to_string(bytes) + " " + selected;
	}

    std::ostringstream out;
    out.precision(1);
    out << std::fixed << count;
    return std::move(out).str() + " " + selected;
}

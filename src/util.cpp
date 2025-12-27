#include <errno.h>

#include <sys/stat.h>
#include <cstring>
#include <error.hpp>
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

int read_entire_file(const char *filename, FILE *fd,
                     std::vector<std::byte> &buffer) {
    long pos;

    if (std::fseek(fd, 0, SEEK_END) || (pos = std::ftell(fd)) == -1 ||
        std::fseek(fd, 0, SEEK_SET)) {
        ERROR("Failed to read %s: %s\n", filename, strerror(errno));
        return -1;
    }

    try {
        buffer.resize(pos);
    } catch (...) {
        ERROR("Failed to read %s. Likely out of memory.\n", filename);
        return -1;
    }

    if (std::fread(buffer.data(), pos, 1, fd) != 1 && std::ferror(fd)) {
        ERROR("Failed to read %s: %s\n", filename, strerror(errno));
        return -1;
    }
    std::clearerr(fd);
    return 0;
}

int open_and_read_entire_file(const char *filename, std::vector<std::byte> &buffer) {
    FILE *fd = nullptr;

    if (!(fd = std::fopen(filename, "r"))) {
        ERROR("Failed to open %s: %s\n", filename, strerror(errno));
        return -1;
    }

    if (read_entire_file(filename, fd, buffer)) {
        std::fclose(fd);
        return -1;
    }

    std::fclose(fd);
    return 0;
}

int write_entire_file(const char *filename, FILE *fd,
                      const std::vector<std::byte> &buffer) {
    if (std::fseek(fd, 0, SEEK_SET) ||
        (std::fwrite(buffer.data(), buffer.size(), 1, fd) != 1 && std::ferror(fd))) {
        ERROR("Failed to write %s: %s\n", filename, strerror(errno));
        return -1;
    }
    std::clearerr(fd);
    return 0;
}

int open_and_write_entire_file(const char                   *filename,
                               const std::vector<std::byte> &buffer) {
    FILE *fd = nullptr;

    if (!(fd = std::fopen(filename, "w"))) {
        ERROR("Failed to open %s: %s\n", filename, strerror(errno));
        return -1;
    }

    if (write_entire_file(filename, fd, buffer)) {
        std::fclose(fd);
        return -1;
    }

    std::fclose(fd);
    return 0;
}

void handle_unknown_option(int optind, char optopt, char **argv) {
    if (optopt) {
        CRIT("Unrecognized option: -%c\n", optopt);
    } else if (argv[optind - 1]) {
        CRIT("Unrecognized option (possibly '%s')\n", argv[optind - 1]);
    } else {
        CRIT("Unrecognized option.\n");
    }
}

void store_uint64_t(uint64_t value, std::vector<std::byte> &data) {
    for (int i = 0; i < 8; i++) {
        data.push_back((std::byte)(value & 0xFF));
        value >>= 8;
    }
}

int restore_uint64_t(std::vector<std::byte>::iterator       &it,
                     const std::vector<std::byte>::iterator &end_it,
                     uint64_t                               &value) {
    if (it + 7 >= end_it) {
        return -1;
    }
    value = 0;
    it = it + 7;

    for (int i = 0; i < 8; i++) {
        value <<= 8;
        value |= (uint64_t)*it;
        it--;
    }
    it += 9;
    return 0;
}

void mkdirr(char *path, mode_t mode) {
    char *ptr = strrchr(path, '/');

    if (!ptr && strlen(path) == 0) {
        return;
    }

    if (ptr) {
        *ptr = '\0';
        mkdirr(path, mode);
        *ptr = '/';
    }

    INFO("Making directory: %s\n", path);
    mkdir(path, mode);

    return;
}

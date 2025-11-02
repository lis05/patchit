#include <errno.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <diff.hpp>
#include <error.hpp>

int NativeDiff::calculate(size_t src_size, std::byte *src_data, size_t dest_size,
                          std::byte *dest_data) {
	// TODO
}

int NativeDiff::from_files(const std::string &src, const std::string &dest) {
    int        r = -1;
    long       src_size, dest_size;
    FILE      *src_fd = nullptr, *dest_fd = nullptr;
    std::byte *src_data = nullptr, *dest_data = nullptr;

    if (!(src_fd = std::fopen(src.c_str(), "r"))) {
        ERROR("Failed to open %s: %s\n", src.c_str(), strerror(errno));
        goto cleanup;
    }

    if (!(dest_fd = std::fopen(dest.c_str(), "r"))) {
        ERROR("Failed to open %s: %s\n", dest.c_str(), strerror(errno));
        goto cleanup;
    }

    if (std::fseek(src_fd, 0, SEEK_END) || (src_size = std::ftell(src_fd)) == -1 ||
        std::fseek(src_fd, 0, SEEK_SET)) {
        ERROR("Could not determine size of %s: %s\n", src.c_str(), strerror(errno));
        goto cleanup;
    }

    if (std::fseek(dest_fd, 0, SEEK_END) ||
        (dest_size = std::ftell(dest_fd)) == -1 ||
        std::fseek(dest_fd, 0, SEEK_SET)) {
        ERROR("Could not determine size of %s: %s\n", dest.c_str(), strerror(errno));
        goto cleanup;
    }

    INFO("%s has size %ld, %s has size %ld\n", src.c_str(), src_size, dest.c_str(),
         dest_size);

    if (!(src_data = (std::byte *)malloc(src_size))) {
        ERROR("Failed to allocate %ld bytes.\n", src_size);
        goto cleanup;
    }

    if (!(dest_data = (std::byte *)malloc(dest_size))) {
        ERROR("Failed to allocate %ld bytes.\n", dest_size);
        goto cleanup;
    }

    if (std::fread(src_data, src_size, 1, src_fd) != 1) {
        ERROR("Failed to read %s: %s\n", src.c_str(), strerror(errno));
        goto cleanup;
    }

    if (std::fread(dest_data, dest_size, 1, dest_fd) != 1) {
        ERROR("Failed to read %s: %s\n", dest.c_str(), strerror(errno));
        goto cleanup;
    }

    INFO("Read files successfully.\n");

    if ((r = calculate((size_t)src_size, src_data, (size_t)dest_size, dest_data))) {
        ERROR("Failed to calculate the difference between %s and %s\n", src.c_str(),
              dest.c_str());
        goto cleanup;
    }

cleanup:
    if (src_fd) {
        std::fclose(src_fd);
    }
    if (dest_fd) {
        std::fclose(dest_fd);
    }
    if (src_data) {
        free(src_data);
    }
    if (dest_data) {
        free(dest_data);
    }

    return r;
}

std::vector<std::byte> NativeDiff::binary_representation() {}
int NativeDiff::from_binary_representation(const std::vector<std::byte> &data) {}
std::vector<std::byte> NativeDiff::apply(const std::vector<std::byte> &data) {}

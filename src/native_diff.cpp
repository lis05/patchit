#include <errno.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <diff.hpp>
#include <error.hpp>

NativeDiff::Record::Record(NativeDiff::RecordType type, uint64_t pos) {
    this->type = type;
    this->pos = pos;
}

int NativeDiff::calculate(size_t src_size, std::byte *src_data, size_t dest_size,
                          std::byte *dest_data) {
    int r = 0;
    changes.clear();

    if (src_size == 0 && dest_size == 0) {
        WARN("No changes, no data.\n");
        return 0;
    }

    size_t *best = NULL, *choice = NULL, prefix_max;

    try {
        /* Handle annoying base cases. */

        if (src_size == 0) {
            INFO("Source size is zero.\n");
            Record rec(NativeDiff::ADD, 0);
            std::get<0>(rec.data).reserve(dest_size);
            for (size_t i = 0; i < dest_size; i++) {
                std::get<0>(rec.data).push_back(dest_data[i]);
            }
            changes.push_back(rec);
            return 0;
        }

        if (dest_size == 0) {
            INFO("Destination size is zero.\n");
            Record rec(NativeDiff::DELETE, 0);
            std::get<1>(rec.data) = src_size;
            changes.push_back(rec);
            return 0;
        }

        /*
         * best[i] = length of the LCS of src and dest ending at and including
         *     dest[i - 1]. best[0] is always zero.
         *
         * best[i] is always in range best[i-1] ... best[i-1] + 1
         */
        best = (size_t *)calloc(dest_size + 1, sizeof(size_t));

        /*
         * choice[i] = position in src such that src[choice[i]] = dest[i] and
         *     the length of LCS is maximal when choosing that combination.
         */
        choice = (size_t *)calloc(dest_size + 1, sizeof(size_t));

        prefix_max = 0;
        for (size_t src_i = 0; src_i < src_size; src_i++) {
            for (size_t dest_i = 0; dest_i < dest_size; dest_i++) {
                if (src_data[src_i] == dest_data[dest_i]) {
                    best[dest_i] = prefix_max + 1;
                    choice[dest_i] = src_i;
                }

                prefix_max = best[dest_i];
            }
        }

        INFO("Longest common subsequence is %zu bytes long.\n", prefix_max);
    } catch (...) {
        ERROR("Likely out of memory.\n");
        r = -1;
    }

    if (best) {
        free(best);
    }

    if (choice) {
        free(choice);
    }

    return r;
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

std::vector<std::byte> NativeDiff::binary_representation() {
}
int NativeDiff::from_binary_representation(const std::vector<std::byte> &data) {
}
std::vector<std::byte> NativeDiff::apply(const std::vector<std::byte> &data) {
}

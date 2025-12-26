#include <unistd.h>

#include <compressor.hpp>
#include <config.hpp>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <diff.hpp>
#include <error.hpp>
#include <util.hpp>
#include <utility>
#include <vector>

static int invoke_tool(const char *tool, const char *command, int ec1, int ec2) {
    INFO("Invoking command `%s`\n", command);
    int r = system(command);
    if (r == -1 || (WEXITSTATUS(r) != ec1 && WEXITSTATUS(r) != ec2)) {
        ERROR(
            "Failed to execute command `%s`, ec=%d, errno: %s. Is `%s` installed?\n",
            command, r, strerror(errno), tool);
        return -1;
    }
    return 0;
}

static const char *const COMMAND_XXD = "xxd -c1 -ps %s > %s";
static const char *const COMMAND_XXD_REVERSE = "xxd -r -c1 -ps %s > %s";
static const char *const COMMAND_DIFF = "diff %s %s > %s";
static const char *const COMMAND_PATCH = "patch -f -s %s %s";

static const char *format(const char *format, ...) {
    va_list list;
    va_start(list, format);
    static char buf[4096];
    vsnprintf(buf, 4096, format, list);
    va_end(list);
    return buf;
}

SystemDiff::SystemDiff() {
    signature = Diff::SYSTEM_DIFF;
}

int SystemDiff::from_files(const std::string &src, const std::string &dest) {
    int r = -1;

    INFO("Constructing SystemDiff from files: %s and %s\n", src.c_str(),
         dest.c_str());

    if (!data.empty()) {
        WARN("Diff data was not empty. Cleaning, but this is weird...\n");
        data.clear();
    }

    char *src_temp = (char *)strdup("/tmp/.patchit.src.XXXXXX");
    char *dest_temp = (char *)strdup("/tmp/.patchit.dest.XXXXXX");
    char *diff_temp = (char *)strdup("/tmp/.patchit.diff.XXXXXX");
    int   fd;

    DEBUG("src_temp: %s, dest_temp: %s, diff_temp: %s\n", src_temp, dest_temp,
          diff_temp);

    if ((fd = mkstemp(src_temp)) == -1 || close(fd)) {
        ERROR("Failed to make a unique temporary file for %s\n", src.c_str());
        goto cleanup;
    }
    INFO("%s will be temporarily written to %s\n", src.c_str(), src_temp);

    if ((fd = mkstemp(dest_temp)) == -1 || close(fd)) {
        ERROR("Failed to make a unique temporary file for %s\n", dest.c_str());
        goto cleanup;
    }
    INFO("%s will be temporarily written to %s\n", dest.c_str(), dest_temp);

    if ((fd = mkstemp(diff_temp)) == -1 || close(fd)) {
        ERROR("Failed to make a unique temporary file for future diff.\n");
        goto cleanup;
    }
    INFO("SystemDiff will be temporarily written to %s\n", diff_temp);

    if (invoke_tool("xxd", format(COMMAND_XXD, src.c_str(), src_temp), 0, 0) ||
        invoke_tool("xxd", format(COMMAND_XXD, dest.c_str(), dest_temp), 0, 0) ||
        invoke_tool("diff", format(COMMAND_DIFF, src_temp, dest_temp, diff_temp), 0,
                    1) == 2) {
        ERROR("Failed to execute the required commands.\n");
        goto cleanup;
    }

    if (open_and_read_entire_file(diff_temp, data)) {
        goto cleanup;
    }

    r = 0;

cleanup:
    if (r) {
        data.clear();
    }
    if (!r) {
        unlink(src_temp);
        unlink(dest_temp);
        unlink(diff_temp);
    } else {
        MSG("To debug the issue, please check the following files:\n");
        MSG("    %s - temporary source file data\n", src_temp);
        MSG("    %s - temporary destination file data\n", dest_temp);
        MSG("    %s - temporary diff data\n", diff_temp);
    }
    free(src_temp);
    free(dest_temp);
    free(diff_temp);

    if (!r) {
        MSG("Created a diff (%s -> %s): %s.\n", src.c_str(), dest.c_str(),
            shorten_size(data.size()).c_str());
    } else {
        ERROR("Failed to create a diff from %s and %s.\n", src.c_str(),
              dest.c_str());
    }
    return r;
}

std::vector<std::byte> SystemDiff::binary_representation() {
    std::vector<std::byte> ret = compressor->compress(data);
    ret.insert(ret.begin(), (std::byte)compressor->getId());
    return ret;
}

// TODO: better error handling
int SystemDiff::from_binary_representation(const std::vector<std::byte> &data) {
    std::vector<std::byte> bytes;

    if (data.empty()) {
        ERROR("Empty data: missing compressor id\n");
        return -1;
    }

    if ((int)data[0] == PlainCompressor::get()->getId()) {
        compressor = PlainCompressor::get();
    } else if ((int)data[0] == ZLibCompressor::get()->getId()) {
        compressor = ZLibCompressor::get();
    } else {
        ERROR("Invalid compressor id: %d\n", (int)data[0]);
        return -1;
    }

    for (size_t i = 1; i < data.size(); i++) {
        bytes.push_back(data[i]);
    }
    this->data = compressor->decompress(bytes);
    return data.size() <= 1 ? !this->data.empty() : this->data.empty();
}

int SystemDiff::apply(const std::string &dest) {
    int r = -1;

    INFO("Applying SystemDiff to %s\n", dest.c_str());
    if (data.empty()) {
        WARN("Empty diff.\n");
    }

    char *diff_temp = (char *)strdup("/tmp/.patchit.diff.XXXXXX");
    char *file_temp = (char *)strdup("/tmp/.patchit.file.XXXXXX");
    int   fd;
    FILE *file = NULL;

    if ((fd = mkstemp(diff_temp)) == -1 || close(fd)) {
        ERROR("Failed to make a unique temporary file for diff: %s\n",
              strerror(errno));
        goto cleanup;
    }
    INFO("SystemDiff will be temporarily written to %s\n", diff_temp);

    if ((fd = mkstemp(file_temp)) == -1 || close(fd)) {
        ERROR("Failed to make a unique temporary file for %s: %s\n", dest.c_str(),
              strerror(errno));
        goto cleanup;
    }
    INFO("%s will be temporarily written to %s\n", dest.c_str(), file_temp);

    if (open_and_write_entire_file(diff_temp, data)) {
        ERROR("Failed to write the diff file %s: %s\n", diff_temp, strerror(errno));
        goto cleanup;
    }
    file = NULL;

    if (invoke_tool("xxd", format(COMMAND_XXD, dest.c_str(), file_temp), 0, 0) ||
        invoke_tool("patch", format(COMMAND_PATCH, file_temp, diff_temp), 0, 0) ||
        invoke_tool("xxd", format(COMMAND_XXD_REVERSE, file_temp, dest.c_str()), 0,
                    0)) {
        ERROR("Failed to execute the required commands.\n");
        goto cleanup;
    }

    r = 0;

cleanup:
    if (file)
        std::fclose(file);
    if (!r) {
        unlink(file_temp);
        unlink(diff_temp);
    } else {
        MSG("To debug the issue, please check the following files:\n");
        MSG("    %s - temporary SystemDiff data\n", diff_temp);
        MSG("    %s - temporary destionation file data\n", file_temp);
    }

    if (!r) {
        MSG("Applied diff to %s\n", dest.c_str());
    } else {
        ERROR("Failed to apply the diff to %s\n", dest.c_str());
    }
    return r;
}

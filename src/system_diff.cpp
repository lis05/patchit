#include <unistd.h>

#include <config.hpp>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <diff.hpp>
#include <error.hpp>
#include <utility>
#include <vector>

SystemDiff::SystemDiff() {
    len = 0;
    data = nullptr;
}

SystemDiff::~SystemDiff() {
    if (data != nullptr) {
        free(data);
    }
}

static int invoke_tool(const char *tool, const char *command, int ec1, int ec2) {
    INFO("Running `%s`\n", command);
    int r = system(command);
    if (r == -1 || (WEXITSTATUS(r) != ec1 && WEXITSTATUS(r) != ec2)) {
        ERROR(
            "Failed executing command `%s`, ec=%d, errno: %s. Is `%s` installed?\n",
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

int SystemDiff::from_files(const std::string &src, const std::string &dest) {
    int r = -1;

    INFO("Constructing SystemDiff from files: %s and %s\n", src.c_str(),
         dest.c_str());

    if (data) {
        free(data);
    }

    char *src_temp = (char *)strdup("/tmp/.patchit.src.XXXXXX");
    char *dest_temp = (char *)strdup("/tmp/.patchit.dest.XXXXXX");
    char *diff_temp = (char *)strdup("/tmp/.patchit.diff.XXXXXX");
    int   fd;
    FILE *file = nullptr;

    DEBUG("src_temp: %s, dest_temp: %s, diff_temp: %s\n", src_temp, dest_temp,
          diff_temp);

    if ((fd = mkstemp(src_temp)) == -1 || close(fd)) {
        ERROR("Failed to make a unique temprorary file for %s\n", src.c_str());
        goto cleanup;
    }

    if ((fd = mkstemp(dest_temp)) == -1 || close(fd)) {
        ERROR("Failed to make a unique temprorary file for %s\n", dest.c_str());
        goto cleanup;
    }

    if ((fd = mkstemp(diff_temp)) == -1 || close(fd)) {
        ERROR("Failed to make a unique temprorary file for future diff.\n");
        goto cleanup;
    }

    DEBUG("src_temp: %s, dest_temp: %s, diff_temp: %s\n", src_temp, dest_temp,
          diff_temp);

    if (invoke_tool("xxd", format(COMMAND_XXD, src.c_str(), src_temp), 0, 0) ||
        invoke_tool("xxd", format(COMMAND_XXD, dest.c_str(), dest_temp), 0, 0) ||
        invoke_tool("diff", format(COMMAND_DIFF, src_temp, dest_temp, diff_temp), 0,
                    1) == 2) {
        ERROR("Failed to execute the required commands.\n");
        goto cleanup;
    }

    long pos;

    if (!(file = std::fopen(diff_temp, "r")) || std::fseek(file, 0, SEEK_END) ||
        (pos = std::ftell(file)) == -1 || std::fseek(file, 0, SEEK_SET)) {
        ERROR("Failed to read the diff: %s\n", strerror(errno));
        goto cleanup;
    }

    len = (size_t)pos;
    INFO("Diff has size of %zu bytes.\n", len);
    if (!(data = (std::byte *)malloc(this->len))) {
        ERROR("Out of memory: %zu bytes.\n", len);
        goto cleanup;
    }

    if (std::fread(data, len, sizeof(std::byte), file) != 1) {
        ERROR("Failed to read the diff: %s\n", strerror(errno));
        goto cleanup;
    }

    r = 0;

cleanup:
    if (file) {
        std::fclose(file);
    }
    if (r && data) {
        free(data);
        data = nullptr;
    }
    unlink(src_temp);
    unlink(dest_temp);
    unlink(diff_temp);
    free(src_temp);
    free(dest_temp);
    free(diff_temp);

    return r;
}

std::vector<std::byte> SystemDiff::binary_representation() {
	std::vector<std::byte> repr(len);
	for (size_t i = 0; i < len; i++) {
		repr[i] = data[i];
	}
	return Config::get()->compressor->compress(repr);
}

// TODO: better error handling
int SystemDiff::from_binary_representation(const std::vector<std::byte> &data) {
	this->data = Config::get()->compressor->decompress(data);
	return data.empty() ? !this->data.empty() : this->data.empty();
}

int SystemDiff::apply(const std::string &file) {
	ASSERT(data != nullptr);
}

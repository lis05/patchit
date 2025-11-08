#pragma once

#include <cstddef>
#include <string>
#include <vector>

std::string shorten_size(size_t bytes);

int open_and_read_entire_file(const char *filename, std::vector<std::byte> &buffer);
int read_entire_file(const char *filename, FILE *fd, std::vector<std::byte> &buffer);

int open_and_write_entire_file(const char *filename, const std::vector<std::byte> &buffer);
int write_entire_file(const char *filename, FILE *fd, const std::vector<std::byte> &buffer);

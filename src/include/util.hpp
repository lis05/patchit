#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

std::string shorten_size(size_t bytes);

int open_and_read_entire_file(const char *filename, std::vector<std::byte> &buffer);
int read_entire_file(const char *filename, FILE *fd, std::vector<std::byte> &buffer);

int open_and_write_entire_file(const char                   *filename,
                               const std::vector<std::byte> &buffer);
int write_entire_file(const char *filename, FILE *fd,
                      const std::vector<std::byte> &buffer);

void handle_unknown_option(int optind, char optopt, char **argv);

void store_uint64_t(uint64_t value, std::vector<std::byte> &data);
int  restore_uint64_t(std::vector<std::byte>::iterator       &it,
                      const std::vector<std::byte>::iterator &end_it,
                      uint64_t                               &value);

/*
 * mkdirs A, A/B, A/B/C for path=A/B/C
 * */
void mkdirr(char *path, mode_t mode);

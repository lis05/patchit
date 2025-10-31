#pragma once

typedef int (*CommandHandler)(int argc, char **argv);

int do_command_create(int argc, char **argv);
int do_command_apply(int argc, char **argv);
int do_command_inspect(int argc, char **argv);

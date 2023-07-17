#pragma once

#include <stdint.h>

int std_install(char *program, char **packages, uint32_t count);

int std_remove(char *program, char **packages, uint32_t count);

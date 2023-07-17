#include "pacman.h"

#include "std_syntax.h"

#define PACMAN "pacman"

int pacman_install(char **packages, uint32_t count) {
	return std_install(PACMAN, packages, count);
}

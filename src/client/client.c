#include "client.h"

#include <stdio.h>
#include <string.h>

#include "alpm.h"

#define PACMAN "pacman"
#define YAY "yay"

static const client *selected;

static const client pacman = {
	.exists = &pmm_alpm_exists,
	.installed = &pmm_alpm_installed,
	.outdated = &pmm_alpm_outdated
};

static const client yay = {
	/* .exists = NULL, */
	.installed = &pmm_alpm_installed,
	/* .outdated = NULL */
};


int client_set(const char *name) {
	if (strcmp(name, PACMAN) == 0) {
		selected = &pacman;
	} else if (strcmp(name, YAY) == 0) {
		selected = &yay;
	} else {
		fprintf(stderr, "unrecognized client: %s\n", name);
		return -1;
	}

	return 0;
}

const client *client_get(void) {
	return selected;
}

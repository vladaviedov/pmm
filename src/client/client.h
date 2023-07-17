#pragma once

#include <stdint.h>

typedef struct {
	// Queries
	int (*exists)(char *name);
	int (*installed)(char *name);
	int (*outdated)(char *name);

	// Actions
	int (*install)(char **packages, uint32_t count);
} client;

int client_set(const char *name);

const client *client_get(void);

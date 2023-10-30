#pragma once

#include <stdint.h>

typedef struct {
	// Queries
	int (*exists)(const char *name);
	int (*installed)(const char *name);
	int (*outdated)(const char *name);

	// Actions
	int (*install)(const char **packages, uint32_t count);
} client;

int client_set(const char *name);

const client *client_get(void);

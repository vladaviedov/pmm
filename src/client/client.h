#pragma once

typedef struct {
	int (*exists)(char *name);
	int (*installed)(char *name);
	int (*outdated)(char *name);
} client;

int client_set(const char *name);

const client *client_get(void);

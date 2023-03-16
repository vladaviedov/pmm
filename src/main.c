#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "commands.h"

typedef struct {
	const char *name;
	int (*func)(int, char **);
} command;

static const command table[] = {
	// Help
	{ "help", &usage },
	{ "--help", &usage },
	{ "-h", &usage },
	// Version
	{ "version", &version },
	{ "--version", &version },
	{ "-v", &version },
	// Add
	{ "add", &add },
	// Remove
	{ "remove", &rm },
	{ "rm", &rm }
};

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("pmm: no options given\n");
		printf("see usage with 'pmm --help'\n");
		return EXIT_FAILURE;
	}

	char *subcommand = argv[1];
	for (uint32_t i = 0; i < sizeof(table) / sizeof(command); i++) {
		if (strcmp(subcommand, table[i].name) == 0) {
			return table[i].func(argc - 2, argv + 2);
		}
	}

	printf("pmm: unknown option '%s'\n", subcommand);
	printf("see usage with 'pmm --help'\n");
	return EXIT_FAILURE;
}

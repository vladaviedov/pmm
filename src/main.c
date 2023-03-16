#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define unused __attribute__((unused))

typedef struct {
	const char *name;
	void (*func)(int, char **);
} subcmd;

void print_usage(int argc, char **argv);
void print_version(int argc, char **argv);

// TODO: remove
void temp_add(int argc, char **argv) {
	printf("temp add reached\n");
	printf("agrc: %d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);
	}
}
void temp_remove(int argc, char **argv) {
	printf("temp remove reached\n");
	printf("agrc: %d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);
	}
}

static const subcmd table[] = {
	// Help
	{ "help", &print_usage },
	{ "--help", &print_usage },
	{ "-h", &print_usage },
	// Version
	{ "version", &print_version },
	{ "--version", &print_version },
	{ "-v", &print_version },
	// Add
	{ "add", &temp_add },
	// Remove
	{ "remove", &temp_remove },
	{ "rm", &temp_remove }
};

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("pmm: no options given\n");
		printf("see usage with 'pmm --help'\n");
		return EXIT_FAILURE;
	}

	char *subcommand = argv[1];
	for (uint32_t i = 0; i < sizeof(table) / sizeof(subcmd); i++) {
		if (strcmp(subcommand, table[i].name) == 0) {
			table[i].func(argc - 2, argv + 2);
			return EXIT_SUCCESS;
		}
	}

	printf("pmm: unknown option '%s'\n", subcommand);
	printf("see usage with 'pmm --help'\n");
	return EXIT_FAILURE;
}

void print_usage(unused int argc, unused char **argv) {
	printf("TODO: print usage\n");
}

void print_version(unused int argc, unused char **argv) {
	printf("TODO: print version\n");
}

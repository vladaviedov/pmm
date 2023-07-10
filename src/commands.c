#include "commands.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "db/pkg.h"

int usage(unused int argc, unused char **argv) {
	printf("usage: pmm [command] <args>\n");
	printf("\n");
	printf("commands:\n");
	printf("\thelp, --help, -h\t\tPrint usage information\n");
	printf("\tversion, --version, -v\t\tPrint pmm version\n");
	printf("\tadd\t\t\t\tAdd package\n");
	printf("\tremove, rm\t\t\tRemove package\n");
	printf("\n");
	printf("see 'pmm [command] --help' for more information\n");

	return EXIT_SUCCESS;
}

int version(unused int argc, unused char **argv) {
	printf("pmm 0.1\n");

	return EXIT_SUCCESS;
}

int add(int argc, char **argv) {
	if (argc < 1) {
		printf("add: no options given\n");
		printf("see usage with 'pmm add --help'\n");
		return EXIT_FAILURE;
	}

	pkg_open();
	int result = pkg_add(argv[0]);
	pkg_save();

	return (result == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
}

int rm(int argc, char **argv) {
	if (argc < 1) {
		printf("remove: no options given\n");
		printf("see usage with 'pmm remove --help'\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int list(int argc, char **argv) {
	if (argc != 0) {
		printf("list: not implemented\n");
		printf("provide no arguments\n");
	}

	pkg_open();
	pkg_print_all();
	pkg_save();

	return EXIT_SUCCESS;
}

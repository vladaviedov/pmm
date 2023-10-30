#include "commands.h"

#include <stdio.h>
#include <stdlib.h>

#include "util/attr.h"
#include "tables/pkg.h"

#define PKG_TABLE "pkg.pmm"

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

	pkg_table *pkgs = pkg_open(PKG_TABLE);
	int result = pkg_add(pkgs, argv[0]);
	if (result < 0) {
		fprintf(stderr, "failed to add package\n");
		return EXIT_FAILURE;
	}

	pkg_save(pkgs);
	return EXIT_SUCCESS;
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
		return EXIT_FAILURE;
	}

	pkg_table *pkgs = pkg_open(PKG_TABLE);
	pkg_check(pkgs);
	pkg_print_all(pkgs);
	pkg_save(pkgs);

	return EXIT_SUCCESS;
}

int sync(int argc, char **argv) {
	if (argc != 0) {
		printf("sync: not implemented\n");
		printf("provide no arguments\n");
		return EXIT_FAILURE;
	}

	pkg_table *pkgs = pkg_open(PKG_TABLE);
	pkg_sync(pkgs);
	pkg_save(pkgs);

	return EXIT_SUCCESS;
}

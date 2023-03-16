#include "commands.h"
#include "common.h"

#include <stdio.h>

void usage(unused int argc, unused char **argv) {
	printf("usage: pmm [command] <args>\n");
	printf("\n");
	printf("commands:\n");
	printf("\thelp, --help, -h\t\tPrint usage information\n");
	printf("\tversion, --version, -v\t\tPrint pmm version\n");
	printf("\tadd\t\t\t\tAdd package\n");
	printf("\tremove, rm\t\t\tRemove package\n");
	printf("\n");
	printf("see 'pmm [command] --help' for more information\n");
}

void version(unused int argc, unused char **argv) {
	printf("pmm 0.1\n");
}

void add(int argc, char **argv) {
	printf("add\n");
}

void rm(int argc, char **argv) {
	printf("rm\n");
}

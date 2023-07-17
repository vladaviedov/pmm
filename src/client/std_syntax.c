#include "std_syntax.h"

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define SYNC "-S"
#define EXPLICIT "--asexplicit"

int std_install(char *program, char **packages, uint32_t count) {
	pid_t pid = fork();

	if (pid < 0) {
		return -1;
	}

	// Child
	if (pid == 0) {
		char *argv[count + 4];

		argv[0] = program;
		argv[1] = SYNC;
		argv[2] = EXPLICIT;

		memcpy(argv + 3, packages, count * sizeof(char *));

		argv[count + 3] = NULL;

		execvp(argv[0], argv);

		// Reaches on exec error
		return -1;
	}

	// Parent
	int result;
	waitpid(pid, &result, 0);
	return WEXITSTATUS(result);
}

#include "std_syntax.h"

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define ROOT_PROG "doas"
#define SYNC "-S"
#define EXPLICIT "--asexplicit"

int std_install(char *program, char **packages, uint32_t count) {
	pid_t pid = fork();

	if (pid < 0) {
		return -1;
	}

	// Child
	if (pid == 0) {
		char *argv[count + 5];

		argv[0] = ROOT_PROG;
		argv[1] = program;
		argv[2] = SYNC;
		argv[3] = EXPLICIT;

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

#include "color.h"

#include <stdio.h>

const char *const ansi_colors[] = {
	// Red
	"\e[0;31m",
	// Yellow
	"\e[0;33m",
	// Green
	"\e[0;32m",
	// White
	"\e[0:37m"
};

void printf_color(color_t color) {
	printf("%s", ansi_colors[color]);
}

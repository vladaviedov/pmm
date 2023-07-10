#pragma once

#define unused __attribute__((unused))

typedef enum {
	RED,
	YELLOW,
	GREEN,
	WHITE
} color_t;

/**
 * @brief Changes printf's output to desired color.
 *
 * @param[in] color - Color to set.
 */
void printf_color(color_t color);

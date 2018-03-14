
#ifndef __VGA__
#define __VGA__

#include <stddef.h>
#include <stdint.h>

static const size_t VGA_BUFFER_ADDRESS = 0x0B8000;

/* Hardware text mode color constants. */
enum vga_color {
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

static const unsigned char VGA_NEWLINE = 0x0A;

// Function prototypes
uint8_t make_color(enum vga_color, enum vga_color);
uint16_t make_vgaentry(char, uint8_t);
size_t strlen(const char *);
void terminal_initialize(void);
void terminal_setcolor(uint8_t);
void terminal_putentryat(char, uint8_t, size_t, size_t);
void terminal_putchar(char);

void terminal_writestring(const char *);

#endif

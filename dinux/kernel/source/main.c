#include <stddef.h>
#include <stdint.h>
 
#include "../../drivers/vga/include/vga.h"

extern void terminal_writestring(const char *);
 
void kernel_main() {
	/* Initialize terminal interface */
	terminal_initialize();
 
	/* Since there is no support for newlines in terminal_putchar
         * yet, '\n' will produce some VGA specific character instead.
         * This is normal.
         */

	terminal_writestring("Let's get this party started.\nHi!");
}

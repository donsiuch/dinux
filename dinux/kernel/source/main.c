 
#include "../../utility/include/io.h"

void kernel_main() {
	/* Initialize terminal interface */
	terminal_initialize();

	int blah = 0xDEADBEEF;

	dumpBytes((unsigned char *)&blah, 4); 
}

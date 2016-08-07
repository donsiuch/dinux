 
#include "../../utility/include/io.h"
#include "../../utility/include/memoryOperations.h"

void kernel_main() {
	/* Initialize terminal interface */
	terminal_initialize();

	dumpBytes((unsigned char *)0x00, 1024); 
}

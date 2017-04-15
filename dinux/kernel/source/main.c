 
#include "../../utility/include/io.h"
#include "../../utility/include/memoryOperations.h"

void kernel_main() {
	/* Initialize terminal interface */
	terminal_initialize();

	//dumpBytes((unsigned char *)0x00, 16); 

	while (1)
	{
	//	printd("In the main loop\n");
		terminal_initialize();
	}
}

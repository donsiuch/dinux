 
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/mm.h"

void kernel_main() {
	/* Initialize terminal interface */
	
	terminal_initialize();
	
	printd("In the main loop\n");

	//getFreeFrame();

	while (1) 
	{
		

	}
}

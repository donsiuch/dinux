 
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/mm.h"

void kernel_main() {
	
	terminal_initialize();
	
	printd("In the main loop\n");

	char *blah_ptr = (char *)0xdeadbeef;

	char blah = *blah_ptr;

	printd("i--> %x", *blah_ptr);

	while (1) 
	{
		

	}
}

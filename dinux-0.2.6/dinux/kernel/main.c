 
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/mm.h"

void kernel_bug(void)
{
    printd("%s(): stack dump:\n", __func__);
    while(1){}
}

void kernel_main() {
	
	terminal_initialize();
    
    

/*
    printd("__kernel_start = 0x%p\n", &__kernel_start);    
    printd("__kernel_end = 0x%p\n", &__kernel_end);
    printd("0xc0100000 pde idx = 0x%p\n", get_pd_idx(0xc0100000));
    printd("a: 0x%p\n", get_pt_idx(0x00000000));
    printd("b:0x%p\n", get_pt_idx(0x00000777));
    printd("c:0x%p\n", get_pt_idx(0x00001000));
    printd("d:0x%p\n", get_pt_idx(0xC0000000)); 
    printd("d:0x%p\n", get_pt_idx(0xC0100000));
    printd("e:0x%p\n", get_pt_idx(0xffffffff));
*/
//	printd("In the main loop\n");
/*
	char *blah_ptr = (char *)0xdeadbeef;

	char blah = *blah_ptr;

	printd("i--> %x", *blah_ptr);
*/

    alloc_page();

	while (1) 
	{
		

	}
}

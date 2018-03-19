 
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/arch_mm.h"

void kernel_bug(void)
{
    printk("%s(): stack dump:\n", __func__);
    while(1){}
}

void kernel_main() {
	
	terminal_initialize();
    
    setup_memory();

/*
    printk("__kernel_start = 0x%p\n", &__kernel_start);    
    printk("__kernel_end = 0x%p\n", &__kernel_end);
    printk("0xc0100000 pde idx = 0x%p\n", get_pd_idx(0xc0100000));
    printk("a: 0x%p\n", get_pt_idx(0x00000000));
    printk("b:0x%p\n", get_pt_idx(0x00000777));
    printk("c:0x%p\n", get_pt_idx(0x00001000));
    printk("d:0x%p\n", get_pt_idx(0xC0000000)); 
    printk("d:0x%p\n", get_pt_idx(0xC0100000));
    printk("e:0x%p\n", get_pt_idx(0xffffffff));
*/

	while (1) {}
}

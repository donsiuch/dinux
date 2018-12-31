 
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/arch_mm.h"

extern void setup_memory(void);
extern struct memory_stats mem_stats;

void kernel_bug(void)
{
    printk("%s(): stack dump:\n", __func__);
    while(1){}
}
extern uint32_t __kernel_size;
void kernel_main() {

    terminal_initialize();

    setup_memory();

    unsigned char *ptr = alloc_page(GFP_KERNEL);

    printk("%p --> %p\n", ptr, *ptr);

    unsigned long val = 0xdeadbeef;
    memcpy(ptr, &val, 4);
    dumpBytes(ptr, 4);

    printk("pages used %p out of %p\n", mem_stats.nr_used_frames, mem_stats.nr_total_frames);

    //memset(0x100000, 0xff, 4);
    //dumpBytes(0xc0105a88, 64);
    //dumpBytes(0x00, 64);

/*
    printk("__kernel_virtual_start + __kernel_size = %p\n", (uint32_t *)((uint32_t)&__kernel_virtual_start + (uint32_t)&__kernel_size));
    printk("*(__kernel_virtual_start + __kernel_size) = %p\n", *(uint32_t *)(uint32_t)&__kernel_virtual_start + (uint32_t)&__kernel_size);
    printk("__kernel_end = %p\n", (&__kernel_end));
    printk("*__kernel_end = %p\n", *(&__kernel_end));
    printk("linker.ld __kernel_size = %p\n", &__kernel_size);
    printk("phys = %p\n", (unsigned int)(&__physical_load_address) + (unsigned int)(&__kernel_size));
    printk("%p\n", *(uint32_t *)(0xc0105A84));
    */
/*
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

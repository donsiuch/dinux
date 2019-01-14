 
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "asm/inc/arch_mm.h"
#include "dinux/inc/vmm.h"

extern void setup_memory(void);
extern struct memory_stats mem_stats;

/*
 * Name: kernel_bug
 *
 * Description: Stops the kernel.
 *
 */ 
void kernel_bug(void)
{
    printk("%s(): stack dump:\n", __func__);
    __asm__ volatile ("hlt;");
}

/*
 * Name: kernel_main
 *
 * Description: Main kernel loop. Never returns. 
 *
 */
void kernel_main() 
{
    terminal_initialize();

    setup_memory();

	setup_heap(0xC0000000);

    unsigned char *ptr = kmalloc(4096, GFP_KERNEL); 

    printk("%p --> %p\n", ptr, *ptr);

		


//    printk("pages used %p out of %p\n", mem_stats.nr_used_frames, mem_stats.nr_total_frames);




    // Kernel never returns from this function.
	while (1) {}
}


 
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "asm/inc/arch_mm.h"
#include "dinux/inc/vmm.h"
#include "asm/inc/buddy.h"

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

    setup_buddy();

    printk("Finished setting up buddy\n");

	//setup_heap();

#if 0
    unsigned char *ptr = kmalloc(8, GFP_KERNEL); 
    printk("%p --> %p\n", ptr, *ptr);
    
    ptr = kmalloc(20, GFP_KERNEL);
    printk("%p --> %p\n", ptr, *ptr);

    ptr = kmalloc(50, GFP_KERNEL);
    printk("%p --> %p\n", ptr, *ptr);
	
    ptr = kmalloc(90, GFP_KERNEL);
    printk("%p --> %p\n", ptr, *ptr);    

    ptr = (unsigned char *)0xc0000000;
    *ptr = 0x77;
	unsigned long virt_addr = boot_kmalloc();
	printk("boot_kmalloc = %p\n", virt_addr);
	
	virt_addr = boot_kmalloc();
	printk("boot_kmalloc = %p\n", virt_addr);

	ptr = (unsigned char *) 0xc0000000;
	*ptr = 0x77;
#endif
    //printk("pages used %p out of %p\n", mem_stats.nr_used_frames, mem_stats.nr_total_frames);

    //allocate_buddy(1);
    //allocate_buddy(1);
    //allocate_buddy(1);


    // Kernel never returns from this function.
	while (1) {}
}


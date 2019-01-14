
#include <stddef.h>

#include "dinux/inc/io.h"
#include "dinux/inc/vmm.h"
#include "arch/asm/inc/arch_mm.h"

static struct heap_index kernel_heap_index_ptr = NULL;

extern void kernel_bug(void);

// TODO: This should be heap code
void *kmalloc(unsigned long size, unsigned long flags)
{
    void *ptr = NULL;

    ptr = alloc_page(flags);

    return ptr;    
}

/*
 * Name: setup_heap
 *
 * Description: Create the heap
 *
 * Arguments:
 *
 */
void setup_heap(unsigned long heap_control_addr)
{
	unsigned long phys_addr = 0;

	phys_addr = pmm_get_free_frame();

	if (phys_addr == 0)
	{	
		printk("%s: Could not find free frame.\n");
		kernel_bug();
	}

	pmm_mark_frame_in_use(phys_addr);
	
	map_virt_to_phys(heap_control_addr, phys_addr);

	// Create the heap index
	kernel_heap_index_ptr = (struct heap_index *)heap_control_addr;

	

printk("AAAA\n");

	while(1){}
}

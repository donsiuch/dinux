
#include <stddef.h>

#include "dinux/inc/io.h"
#include "dinux/inc/vmm.h"
#include "arch/asm/inc/arch_mm.h"
#include "dinux/inc/memory.h"

static struct heap_index *heap_idx_ptr = NULL;
static struct vm_area *virt_mem_map_ptr = NULL;

unsigned long unused_virt_addr_ptr = 0;

extern void kernel_bug(void);

struct vm_area * find_vm_area_by_addr(unsigned long addr)
{
    unsigned long start;
    unsigned long size;

    if(virt_mem_map_ptr == NULL)
    {
        return NULL;
    }

    while (virt_mem_map_ptr != NULL)
    {
        start = virt_mem_map_ptr->start;
        size = virt_mem_map_ptr->size;

        if (addr >= start && addr < (start + size))
        {
            return virt_mem_map_ptr;
        }

        virt_mem_map_ptr = virt_mem_map_ptr->next_ptr;
    }

    return NULL;
}

unsigned long vmm_get_free_virt_addr()
{
    unsigned long addr = (unsigned long)unused_virt_addr_ptr;

    //
    // TODO: While we are not at the end of memory
    //

    unused_virt_addr_ptr += PAGE_SIZE;

    return addr;
}

/*
 * Name:        alloc_page	
 *
 * Description: Get the virtual address of a new page.	
 *
 * Arguments: 	void
 *
 * Return:	Success - Virtual address of new page
 *		    Failure - NULL
 *
 */
void * alloc_page(unsigned long flags)
{
    unsigned long phys_addr = 0;
    unsigned long virt_addr = 0;

    phys_addr = pmm_get_free_frame();

    pmm_mark_frame_in_use(phys_addr);

    virt_addr = vmm_get_free_virt_addr();

    // TODO: If this fails, don't go any further.
    if (flags & GFP_KERNEL)
    {
        map_virt_to_phys(virt_addr, phys_addr);
    }
    
    return (void *)virt_addr;
}

unsigned long grab_from_heap(unsigned long size)
{
    int i;

    struct chunk_head *chunk_ptr;

//
// TODO: LOCK THE HEAP
//

    //
    // TODO: Can we get to the heap we need quicker than O(n)?
    //
    for (i = 0; i < VMM_MAX_HEAP_IDX; i++)
    {
        printk("size = %p v size = %p\n", heap_idx_ptr->heap_head[i]->size,size);

        if (heap_idx_ptr->heap_head[i]->size > size)
        {

            //
            // Grab from the top of the list
            //
            chunk_ptr = heap_idx_ptr->heap_head[i]->free_chunk_ptr;

            //
            // Update the heap to point to the next free chunk
            //
            heap_idx_ptr->heap_head[i]->free_chunk_ptr = VMM_CHUNK_DATA(chunk_ptr)->next_ptr;

            //
            // Since we are the top of the list, set next chunk's prev to NULL
            //
            VMM_CHUNK_DATA(VMM_CHUNK_DATA(chunk_ptr)->next_ptr)->prev_ptr = NULL;

            printk("%s: Allocated %p Bytes from the kernel heap.\n", __func__, size);

//
// TODO: UNLOCK THE HEAP
//

            return (unsigned long)chunk_ptr;
        }
    } 

    printk("%s: Failed to get memory chunk from the heap\n", __func__);

    kernel_bug();

    return 0;
}

//
//
//
void *kmalloc(unsigned long size, unsigned long flags)
{
    void *ptr = NULL;

    if (flags & GFP_KERNEL)
    {
        ptr = (void *)grab_from_heap(size);
    }
    else
    {
        printk("%s: Unknown flags, %p\n", __func__, flags);
        kernel_bug();
    }

    return ptr;    
}

//
// c_ptr is assumed to point to block of PAGE_SIZE
//
void initialize_heap(struct chunk_head *chunk_ptr, unsigned long chunk_size)
{
    int i;
    struct chunk_head *current_ptr = chunk_ptr;
    struct chunk_head *prev_ptr = NULL;
    unsigned long single_chunk_total_size = chunk_size + sizeof(struct chunk_head);
    int max_idx = PAGE_SIZE/single_chunk_total_size;

    memset(current_ptr, 0, sizeof(PAGE_SIZE));

    for (i = 0; i < max_idx; i++)
    {
        current_ptr->size = chunk_size;
        current_ptr->magic = VMM_CHUNK_IN_USE;
    
        VMM_CHUNK_DATA(current_ptr)->prev_ptr = prev_ptr;
        VMM_CHUNK_DATA(current_ptr)->next_ptr = (struct chunk_head *)((unsigned long)current_ptr + single_chunk_total_size);

        prev_ptr = current_ptr;

        current_ptr = VMM_CHUNK_DATA(current_ptr)->next_ptr;
    }

    printk("There are %p chunks\n", i); 
}

int power(int base, int power)
{
    int i;
    int ret = base;

    if (power == 0)
    {
        return 1;
    }

    for (i = 1; i < power; i++)
    {
        ret = ret * base;
    }   

    return ret;
}

/*
 * Name: setup_heap
 *
 * Description: Create the heap
 *
 * Arguments:
 *
 */
void setup_heap()
{
    int i;
    struct heap_head *heap_head_ptr;

	// Create the heap index
	heap_idx_ptr = (struct heap_index *)alloc_page(GFP_KERNEL);

    printk("heap_idx_ptr = %p\n", heap_idx_ptr);

    //
    // In the addresses immediately after the heap index the heap_head's will
    // follow
    //
    heap_head_ptr = (struct heap_head *)((unsigned long)heap_idx_ptr + sizeof(struct heap_index));
    
    for (i = 0; i < VMM_MAX_HEAP_IDX; i++)
    {
        heap_idx_ptr->heap_head[i] = heap_head_ptr;
        heap_head_ptr = (struct heap_head *)((unsigned long)heap_head_ptr + sizeof(struct heap_head));
        //printk("heap[%p] = %p\n", i, heap_idx_ptr->heap_head[i]);
    } 

    //
    // Allocate the actual heaps.
    //
    for (i = 0; i < VMM_MAX_HEAP_IDX; i++)
    {
        heap_idx_ptr->heap_head[i]->free_chunk_ptr = (struct chunk_head *)alloc_page(GFP_KERNEL);

        //printk("heap[%p]->free_ptr = %p\n", i, heap_idx_ptr->heap_head[i]->free_chunk_ptr);

        heap_idx_ptr->heap_head[i]->size = power(2, i + 4);

        initialize_heap(heap_idx_ptr->heap_head[i]->free_chunk_ptr, power(2, i + 4));
    }

    printk(">> %p\n", heap_idx_ptr->heap_head[0]->size);
    //dumpBytes(heap_idx_ptr->heap_head[0]->free_chunk_ptr, 64);
}


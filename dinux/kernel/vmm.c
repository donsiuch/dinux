
#include <stddef.h>

#include "dinux/inc/vmm.h"
#include "arch/asm/inc/arch_mm.h"

// TODO: This should be heap code
void *kmalloc(unsigned long size, unsigned long flags)
{
    void *ptr = NULL;

    ptr = alloc_page(flags);

    return ptr;    
}


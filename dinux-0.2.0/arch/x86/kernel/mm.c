
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/mm.h"

// Get a physical page frame

void setupPaging()
{
	int x = 0;
	uint32_t frameAddress = 0x00000000;

	// Clear the frame bitmap
	//memset(frameLedger, 0, sizeof(frameLedger));

	// Create the kernel page directory and clear it
	kernel_pd = (uint32_t *)KERNEL_PD_ADDR;
	memset(kernel_pd, 0, PAGE_SIZE);

	// Create the identity mapped page table
	pt_ident = (uint32_t *)PT_IDENT_ADDR;
	memset(pt_ident, 0, PAGE_SIZE);

	// Hook up identity page table to the page directory
	kernel_pd[0] = (uint32_t)pt_ident;
	kernel_pd[0] |= 1;
	kernel_pd[0] |= 2;

	// Identy map the first 32 pages
	while ( x < 1024 )
	{
		pt_ident[x] = (uint32_t)frameAddress;
		pt_ident[x] |= 1;
		pt_ident[x] |= 2;

		frameAddress += PAGE_SIZE;
		x++;	
	}	
}

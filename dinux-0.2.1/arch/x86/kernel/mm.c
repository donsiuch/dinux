
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/mm.h"

// Bitmap that describes used/free physical page frames
// Each bit of the BITMAP_UNIT represents a physical frame
// a '1' means the frame is used. A '0' means frame is free.
static BITMAP_UNIT frameLedger[NUM_LEDGER_UNITS];

SMAP_entry_t smapBuffer[16];

/*
 * Name: 	setFrameInUse
 *
 * Abstract: 	Given frame index, mark in use by setting bit
 *		to 1.
 *
 * Arguments: 	Frame index	
 *
 * Return:	void
 *
 */
void setFramInUse()
{

}

/*
 * Name: 	getFirstFreeIndex()
 *
 * Abstract: 	Scan the frame ledger for the first non-set bit.
 *
 * Arguments: 	void	
 *
 * Return:	Success	Index of the first 0 bit (corresponds to free frame)
 *		Failure -1
 *
 * This method is very slow and needs to be replaced with a
 * more efficient algorithm to manage physical memory.
 *
 */
int getFirstFreeIndex()
{
	int x = 0;
	int y;
	BITMAP_UNIT testBit;

	// Scan every page frame(LSB -> MSB), testing each bit of the ledger
	// for a free frame. If a 0 is found, the frame is free.
	while (x < NUM_LEDGER_UNITS){

		y = 0;
		testBit = 0x01;	

		while (y < PAGES_PER_UNIT){

			// Test the MSB -> LSB 
			if (!(frameLedger[x] & testBit)){
				printd("Found free frame. x = %p, y = %p\n", x, y);
				return ((x*PAGES_PER_UNIT)+y);
			}
		
			// Debugging	
			//printd("Frame = %p, testBit = %p, result = %p\n", frameLedger[x], testBit, frameLedger[x] & testBit);

			testBit = testBit << 1;
			y++;
		}
		x++;
	}	

	printd("Failed to find free frame.\n");
	
	return -1;
}

/*
 * Name: 	getFreeFrame
 *
 * Abstract: 	Returns physical address of frame
 *
 * Arguments: 	void
 *
 * Return:	Success - Address of physical frame
 *		Failure - ??? -- We probably need to die.
 *
 */
void * getFreeFrame()
{
	getFirstFreeIndex();
	return NULL;
}

void setupPaging()
{
	int x = 0;
	uint32_t frameAddress = 0x00000000;

	// Clear the frame bitmap
	memset(frameLedger, 0, sizeof(frameLedger));

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
	while ( x < 1024 ){

		pt_ident[x] = (uint32_t)frameAddress;
		pt_ident[x] |= 1;
		pt_ident[x] |= 2;

		frameAddress += PAGE_SIZE;
		x++;	
	}	

    // Clear page table for the kernel
    pt_kernel = (uint32_t *)PT_KERNEL_ADDR;
    memset(pt_kernel, 0, PAGE_SIZE);

    // Start mapping pages at the 0xc0000000 page table
    // 0xc0000000/(4096*1024) = 0x300 = 768
    kernel_pd[768] = (uint32_t)pt_kernel;
    kernel_pd[768] |= 1;
    kernel_pd[768] |= 2;

    x = 0;
    frameAddress = 0x100000;
    while (x < 1024){
        pt_kernel[x] = (uint32_t)frameAddress;
        pt_kernel[x] |= 1;
        pt_kernel[x] |= 2;
        frameAddress += PAGE_SIZE;
        x++;
    }
}

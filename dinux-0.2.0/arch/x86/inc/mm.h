
#ifndef __MM_H__
#define __MM_H__

#include <stdint.h>

#define	PAGE_SIZE	0x1000
#define MEMORY_SIZE	0x1000000
#define BITMAP_UNIT	uint8_t

// Proof of concept paging
#define KERNEL_PD_ADDR	0x1e000
#define	PT_IDENT_ADDR	0x1f000
#define NUM_IDENT_PAGES	0x20

// Page table entry
typedef struct __attribute((packed)) {
	// Must be 1 to map a page
	uint32_t present	: 1;

	// 1 == r+w, 0 == r
	uint32_t rw		: 1;

	// 0 == supervisor ( user cannot access )
	uint32_t user		: 1;

	// Write through
	uint32_t wt		: 1;

	// Cache disable 
	uint32_t cd		: 1;

	uint32_t accessed	: 1;
	uint32_t dirty		: 1;

	// Pat support
	uint32_t pat		: 1;
	
	// If CR4.PGE = 1, determines if translation is global
	// else ignored
	uint32_t global		: 1;
	uint32_t unused0	: 3;

	// Since the most significant 20 bits... 1000
	uint32_t frameAddress	: 20;
} pte;

// Page directory entry
typedef struct __attribute((packed)) {
	uint32_t present	: 1;
	uint32_t rw		: 1;
	uint32_t user		: 1;
	uint32_t wt		: 1;
	uint32_t cd		: 1;
	uint32_t accessed	: 1;
	uint32_t dirty		: 1;

	// Page size, 0 for 4096
	uint32_t ps		: 1;
	uint32_t unused0	: 4;

	// Since the most significant 20 bits... 1000
	uint32_t pageTableAddress : 20;

} pde;

uint32_t *kernel_pd = NULL;
uint32_t *pt_ident = NULL;

// Bitmap that describes used/free physical page frames
// 	mem_size/page_size/uint32_t/
//BITMAP_UNIT frameLedger[MEMORY_SIZE/PAGE_SIZE/sizeof(BITMAP_UNIT)];

void	setupPaging();

#endif

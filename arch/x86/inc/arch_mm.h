
#ifndef __ARCH_MM__
#define __ARCH_MM__

/*
 * Memory layout
 * 0x00000000+---------------+
 *           |  Identity     |
 *           |   Mapped      |
 *           |    Region     |
 *           |               |
 * 0x9000    | meme820       |
 *           |               |
 * 0x0001e000| Boot Page D.  |
 * 0x0001f000| Boot Page T.  |
 * 0x00020000| End Mem Map   |
 *           |               |
 *           +---------------+
 *
 */

#ifndef ASSEMBLY

#include <stdint.h>

extern uint32_t __kernel_virtual_start;
extern uint32_t __kernel_start;
extern uint32_t __kernel_end;
extern uint32_t __physical_load_address;

#define	PAGE_SIZE	0x1000
#define PAGE_SHIFT  12
#define MEMORY_SIZE	0x1000000
#define TOTAL_NUM_PAGES	(MEMORY_SIZE/PAGE_SIZE)

// 1 Byte ( 8 bits ) will be the data type
// for the array. Each Byte can track
// 8 physical, contiguous frames
#define BITMAP_UNIT		uint8_t
#define PAGES_PER_BITMAP_UNIT	8
#define PAGES_PER_UNIT		(1*PAGES_PER_BITMAP_UNIT)
#define NUM_LEDGER_UNITS 	(TOTAL_NUM_PAGES/PAGES_PER_UNIT)

// Page table entry
typedef struct {
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
	uint32_t frame_addr	: 20;
} 
__attribute((packed)) 
pte_t;

// Page directory entry
typedef struct {
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
	uint32_t pt_addr : 20;

} 
__attribute((packed)) 
pde_t;

// Represents a region a physical memory
struct mem_node {
    struct mem_node *next;
    unsigned long start_addr;
    unsigned long end_addr;
}
__attribute((packed));

void set_frame_in_use(BITMAP_UNIT *, uint32_t);
int getFirstFreeIndex(void);
unsigned long alloc_page(void);
void	setupPaging(void);
unsigned long 	get_free_frame(void);
uint32_t get_pd_idx(uint32_t);
uint32_t get_pt_idx(uint32_t);
void setup_memory(void);

#endif	// #ifndef ASSEMBLY

// Proof of concept paging
#define HIGH_MEM_START_ADDR 0xc0000000
#define PAGING_BIT	0x80000000
#define KERNEL_PD_ADDR	0x1d000
#define	PT_IDENT_ADDR	0x1e000
#define PT_KERNEL_ADDR  0x1f000 

// Page directory entry 
#define PT_PRESENT  0x01
// Read + write 
#define PT_RW       0x02

// Page table entry
#define PAGE_PRESENT    0x01
#define PAGE_RW         0x02

#define PAGE_ALIGN(_x)(_x &= 0xfffff000)
#define GET_FRAME_ADDR PAGE_ALIGN

#endif	// __ARCH_MM__

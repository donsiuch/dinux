
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

// TODO: These are used in assembly files
#define HIGH_MEM_START_ADDR 0xc0000000
#define PAGING_BIT	0x80000000
#define KERNEL_PD_ADDR	0x1d000

#ifndef ASSEMBLY

#include <stdint.h>

#include "dinux/inc/list.h"

// These macros are most useful when dealing with
// memory at HIGH_MEM_START_ADDR+
#define VIRTUAL_OFFSET HIGH_MEM_START_ADDR

extern uint32_t __kernel_virtual_start;
extern uint32_t __kernel_start;
extern uint32_t __kernel_end;
extern uint32_t __physical_load_address;

#define	PAGE_SIZE	0x1000
#define PAGE_SHIFT_SIZE  12
#define MEMORY_SIZE	0x1000000
#define TOTAL_NUM_PAGES	(MEMORY_SIZE/PAGE_SIZE)
#define NUM_PD_ENTRIES (PAGE_SIZE/sizeof(uint32_t))
#define NUM_PT_ENTRIES NUM_PD_ENTRIES
#define TOTAL_MEM_REGION_PER_PT (NUM_PT_ENTRIES*PAGE_SIZE)

// "Get free page" -- for allocating pages
#define GFP_KERNEL  0x00000001
#define GFP_USER    0x00000020

#define SELF_MAP_ADDR 0xFFC00000

#define	PT_IDENT_ADDR	0x1e000
#define PT_KERNEL_ADDR  0x1f000 

// Page directory entry 
#define PT_PRESENT  0x01
// Read + write 
#define PT_RW       0x02

// Page table entry
#define PAGE_PRESENT    0x01
#define PAGE_RW         0x02

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
	uint32_t pt_phys_addr : 20;

} 
__attribute((packed)) 
pde_t;

struct memory_stats {
    unsigned long nr_total_frames;
    unsigned long nr_used_frames;
};

// The size of struct page must always divide evenly into PAGE_SIZE. This is
// because certain boot functions assume an even division when setting up page
// tables and mapping in the physical frame ledger.
struct page {
    struct list_head list;
    unsigned long count;
}
__attribute((packed));

#define MEM_ZONE_DMA_MAX_ADDR 0x1000000
typedef enum {

    // <= 0x01000000
    ZONE_DMA = 0,

    // > 0x01000000
    ZONE_NORMAL = 1,
    ZONE_MAX_NR,
} ZONE_T;

#define MEM_MAX_ORDER 5
struct mem_zone {
    //
    // [0] = 2^0 = 1x page
    // [1] = 2^1 = 2x pages
    // [2] = 2^2 = 4x pages
    // etc.
    //
    struct list_head *free_list[MEM_MAX_ORDER]; 
};

struct mem_node {
    struct mem_zone mem_zone[ZONE_MAX_NR];
};

inline unsigned long PAGE_ALIGN(unsigned long addr)
{
    return (addr &= 0xFFFFF000);
}

// TODO: Currently unused
inline unsigned long PAGE_SHIFT(unsigned long addr)
{
    return (addr << PAGE_SHIFT_SIZE);
}

inline unsigned long VIRT_TO_PHYS(unsigned long addr)
{
    return (addr - VIRTUAL_OFFSET);
}

inline unsigned long PHYS_TO_VIRT(unsigned long addr)
{
    return (addr + VIRTUAL_OFFSET);
}

inline unsigned long CREATE_PTE(uint32_t addr, uint32_t flags)
{
    return (addr | flags);
}
#define CREATE_PDE CREATE_PTE

int getFirstFreeIndex(void);
void * alloc_page(unsigned long);
void	setupPaging(void);
unsigned long 	get_free_frame(void);
uint32_t get_pd_idx(uint32_t);
uint32_t get_pt_idx(uint32_t);
void setup_memory(void);
void pmm_mark_frame_in_use(unsigned long);
int is_page_mapped(pde_t *, uint32_t );
int is_pt_present(unsigned long);
int is_pg_present(pte_t *, uint32_t );
void map_virt_to_phys(unsigned long, unsigned long);
void unmap_virt(unsigned long);
void install_page_table(unsigned long, unsigned long);
void install_page(unsigned long, unsigned long);
int is_page_present(unsigned);
unsigned long pmm_get_free_frame(void);

#endif	// #ifndef ASSEMBLY

#endif	// __ARCH_MM__

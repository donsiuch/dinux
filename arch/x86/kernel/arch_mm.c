
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/meme820.h"
#include "x86/inc/arch_mm.h"

// Bitmap that describes used/free physical page frames
// Each bit of the BITMAP_UNIT represents a physical frame
// a '1' means the frame is used. A '0' means frame is free.
static BITMAP_UNIT frameLedger[NUM_LEDGER_UNITS];

static uint32_t total_num_pte_per_pt = PAGE_SIZE/sizeof(pte_t);

// Physical memory map
struct mem_node *mem_node_ptr = NULL;

// Reference kernel page directory
pde_t *ref_pgd;

extern void kernel_bug(void);

/*
 * Name: 	    set_frame_in_use
 *
 * Abstract: 	Given frame index, mark in use by setting bit to 1.
 *              
 *              TODO: Die appropriately if things go wrong in this function.
 *
 * Arguments: 	Frame index	
 *
 * Return:	    void
 *
 */
void set_frame_in_use(BITMAP_UNIT *f_ledger_ptr, uint32_t phys_addr)
{
    uint32_t f_ledger_idx;
    uint32_t f_offset_into_idx;

    // Start with the LSB 
    uint8_t which_frame = 0x01;

    if (f_ledger_ptr == NULL)
    {
        printk("%s(): f_ledger_ptr = 0x%p\n", __func__, f_ledger_ptr);
        kernel_bug(); 
    }

    // Which ledger indice
    f_ledger_idx = phys_addr/(PAGES_PER_BITMAP_UNIT * PAGE_SIZE);

    // Which bit
    f_offset_into_idx = (phys_addr >> PAGE_SHIFT)%PAGES_PER_BITMAP_UNIT;

    if (f_offset_into_idx > 0)
    {
        which_frame <<= f_offset_into_idx;
    }

    // Verify we are not overwriting a page.
    if (f_ledger_ptr[f_ledger_idx] & which_frame)
    {
        printk("%s(): Frame in use! 0x%p, 0x%p, 0x%p\n", __func__, f_ledger_idx, f_offset_into_idx, which_frame);
        kernel_bug(); 
    }

    f_ledger_ptr[f_ledger_idx] |= which_frame;
}

/*
 * Name: 	getFirstFreeIndex()
 *
 * Abstract: 	Scan the frame ledger for the first non-set bit.
 *
 * Arguments: 	void	
 *
 * Return:	Success	Index of the first 0 bit (corresponds to free frame)
 *		    Failure -1
 *
 * TODO: This method is very slow and needs to be replaced with a
 * more efficient algorithm to manage physical memory.
 *
 * TODO: This function starts scanning the physical frame ledger starting
 * from __physical_load_address to reduce the complexity. Eventually this
 * should scan after the meme820 map has been sanitized. Things to consider:
 * this function should be able to find frames based on the type of memory:
 * - Normal kmalloc() and similar
 * - Memory that requires a specific region
 *
 */
int getFirstFreeIndex(void)
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
				printk("Found free frame. x = %p, y = %p\n", x, y);
				return ((x*PAGES_PER_UNIT)+y);
			}
		
			// Debugging	
			//printk("Frame = %p, testBit = %p, result = %p\n", frameLedger[x], testBit, frameLedger[x] & testBit);

			testBit = testBit << 1;
			y++;
		}
		x++;
	}	

	printk("Failed to find free frame.\n");
	
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
 *		    Failure - ??? -- We probably need to die.
 *
 */
unsigned long get_free_frame(void)
{
    int32_t idx = getFirstFreeIndex();
    if (idx < 0)
    {
        printk("%s(): Failed to find a free frame!\n", __func__);
        kernel_bug();
    }

    return (idx*PAGE_SIZE);
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
unsigned long alloc_page(void)
{
    unsigned long phys_addr = 0;
    int i;

    for (i = 0; i < 16; i++)
    {
        phys_addr = get_free_frame();
       // printk("Free adress: 0x%p ledger before = 0x%p\n", phys_addr, frameLedger[0]);
        set_frame_in_use(frameLedger, phys_addr);
        printk("Frame ledger after: 0x%p \n", frameLedger[(i/8)]);
    }

    return 0;
}
extern uint32_t __kernel_size;
/*
 *
 * This is run in protected mode before
 * paging is set up
 *
 */
void setup_memory(void)
{
    int total_nr_pages = 0;

	// Clear the frame bitmap
	//memset(frameLedger, 0, sizeof(frameLedger));

    // Organize and find out how much memory we have
    sanitize_meme820_map();

    total_nr_pages = get_total_nr_pages();
    if(total_nr_pages <= 0)
    {
        kernel_bug();
    }

    printk("Need to make an array = 0x%p Bytes long\n", total_nr_pages/8);

	// Set the physical frame manager. 
    printk("Start of kernel = 0x%p, End of kernel = 0x%p\n", &__kernel_start, &__kernel_end);
    int x = (int)get_pt_idx((uint32_t)0xc0100000);
    printk(">> %p\n", x);
    uint32_t *pt_kernel = PT_KERNEL_ADDR; 
    while (pt_kernel[x] != 0){
        printk(">>> %p is populated!\n", pt_kernel[x]);
        x++;
    }

    //uint32_t *ptr = (uint32_t *)((uint32_t)(&__physical_load_address) + (uint32_t)(&__kernel_size));
    //*ptr = (uint32_t)0xdeadbeef;
    //memset(0x0000, 0xaa, 4);

	// Allocate frame for identity table
	// 


}

/* Name: get_pt_idx
 *
 * Description: Given a virtual address, calculate the
 *              index into the page table that the
 *              virtual address will map to.
 *
 *              Denominators are assumed to never be zero.
 *
 *  Arguments:  virt_addr
 *
 *  Returns:    Index in pt that that maps the virt_addr
 *
 */
uint32_t get_pt_idx(uint32_t virt_addr)
{
    uint32_t result = 0x00;

    // Strip away the page directory
    result = virt_addr%(PAGE_SIZE*total_num_pte_per_pt);

    // Result holds the number of Bytes offset into the pt.
    //
    // Find the page that contains this virtual address by page aligning the
    // address.
    PAGE_ALIGN(result); 
    
    // Since each pte maps 1024 (0x400) Bytes, divide the 
    // offset into the pt by 1024 to find the pte index
    // that the virtual address should be mapped to.
    return (result/PAGE_SIZE);
}

/*
 * Name:        get_pd_idx
 *
 * Description: Given a virtual address, calculate the
 *              index in the page directory that the
 *              virtual address will be map to.
 *
 *              Denominator is assumed to never be zero.
 *
 * Arguments:   virt_addr
 *
 * Returns:     Index in pd that maps virt_addr
 *
 */
uint32_t get_pd_idx(uint32_t virt_addr)
{
    // Each pde points to a page table that map address the
    // denominator calculation:
    //  Number of pte per pt where each entry maps a PAGE_SIZE
    //  region.
    //
    //  Example:
    //  0xc0000000/(0x1000*0x400) = 768
    return (virt_addr/(PAGE_SIZE*total_num_pte_per_pt));
}

/* 
 * Name:        create_pte
 *
 * Description: Create 
 *
 */
/*
uint8_t create_pte(pde_t *pgd_ptr, uint32_t virt_addr, uint32_t pg_phys_addr, uint32_t flags)
{
    uint32_t pd_idx;
    uint32_t pt_idx;    
    pte_t *pt_ptr;

    if (pgd_ptr == NULL)
    {
        return 1;
    }

    pd_idx = get_pd_idx(virt_addr);

    // TODO:
    //
    // Is there a page table there?
    // 
    // if not
    //  allocate_new_page   
    //  
    //  hook up correct entry in the pgd

    pt_idx = get_pt_idx(virt_addr);

    // TODO:
    //
    // Is there a page table entry there?
    // 
    // if not
    //  allocate_new_page 

    // TODO: This might be unnecessary
    PAGE_ALIGN(virt_addr);

    // When I do pgd_ptr[idx].pt_addr does it get 32 Bytes, 20?
    //printk("---> %p\n", pgd_ptr[pd_idx]);
        
    //pt_ptr = (pte_t *)(PAGE_ALIGN(pgd_ptr[pd_idx].pt_addr));

    //pt_ptr[pt_idx] = pg_phys_addr;

    //pt_ptr[pt_idx] |= flags;
    

    // We don't set the frame in use in this function, but it should be set

    // TODO: Fix return
    return 0;    
}
*/

// This function is called from low memory identity mapped code
// to help get into paging mode successfully. 
/* Name:        setupPaging
 *
 * Abstract:    Initializes page tables used for booting.
 *
 * Description: This function is called during boot when th process is in real
 *              mode. The purpose is to initialize the page tables for to prep for 
 *              the switch to protected mode.
 *              
 *              This function has two main parts:
 *                  Identity map 0x00000000 - 0x00400000
 *                      - This region contains the boot pd and pt
 *                      - Real mode code
 *                      - Terminal buffer
 *                      - BIOS code (must be accessed from real mode though...)
 *
 *                  Map __kernel_start virtual address to where it is actually
 *                  loaded in memory.
 *
 *  Arguments:  void
 *
 */
void setupPaging()
{
	int x = 0;
	uint32_t frameAddress = 0x00000000;
    uint32_t virt_addr = 0x00000000;
    uint32_t kernel_start = (uint32_t)(&__kernel_start);
    uint32_t kernel_end = (uint32_t)(&__kernel_end);
    uint32_t physical_load_addr = (uint32_t)(&__physical_load_address);

	// These symbol names are also global but cannot be accessed
	// because their virtual address is in high memory. 
	//
	// In the meantime, declare locally and use position independent,
	// hard coded addresses to get us into paging mode. 
	uint32_t *kernel_pd;
	uint32_t *pt_ident;
	uint32_t *pt_kernel;

	// Create the kernel page directory and clear it
	kernel_pd = (uint32_t *)KERNEL_PD_ADDR;
	memset(kernel_pd, 0, PAGE_SIZE);

    //
    // Part 1: Identity mapping
    //
	// Create the identity mapped page table
	pt_ident = (uint32_t *)PT_IDENT_ADDR;
	memset(pt_ident, 0, PAGE_SIZE);

    // Calculate the pd index for the identity mapping
    // starting at 0x00000000
    x = get_pd_idx(frameAddress);

	// Hook up identity page table to the page directory
	kernel_pd[x] = (uint32_t)pt_ident;
	kernel_pd[x] |= PT_PRESENT;
	kernel_pd[x] |= PT_RW;

	// Identy map alot of the kernel: (0x00000000 -> 0x00400000)
    // TODO: This can probably be reduced.
    x = 0;
	while ( x < 1024 ){
		pt_ident[x] = (uint32_t)frameAddress;
		pt_ident[x] |= PAGE_PRESENT;
		pt_ident[x] |= PAGE_RW;

		frameAddress += PAGE_SIZE;
		x++;	
	}	

    //
    // PART 2 - Kernel virtual address mapping
    //
	// Create the page table for the kernel 
	pt_kernel = (uint32_t *)PT_KERNEL_ADDR;
	memset(pt_kernel, 0, PAGE_SIZE);

    // Calculate pd index for the virtual address mapping
    // for the kernel
    x = get_pd_idx(kernel_start);

	// Start mapping pages at the 0xc0100000 page table
	// 0xc0100000/(4096*1024) = 0x300 = 768
	kernel_pd[x] = (uint32_t)pt_kernel;
	kernel_pd[x] |= PT_PRESENT;
	kernel_pd[x] |= PT_RW;

	x = get_pt_idx(kernel_start);
    virt_addr = kernel_start;
	frameAddress = physical_load_addr;
	while (virt_addr < kernel_end){
		pt_kernel[x] = (uint32_t)frameAddress;
		pt_kernel[x] |= PAGE_PRESENT;
		pt_kernel[x] |= PAGE_RW;

		frameAddress += PAGE_SIZE;
        virt_addr += PAGE_SIZE;
        x++;
	}
}

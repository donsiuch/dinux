
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/meme820.h"
#include "x86/inc/arch_mm.h"

// Bitmap that describes used/free physical page frames
// Each bit of the BITMAP_UNIT represents a physical frame
// a '1' means the frame is used. A '0' means frame is free.
// TODO: Remove all traces of the frame ledger.
static BITMAP_UNIT frameLedger[NUM_LEDGER_UNITS];

// Array of struct pages that describe each page of physical memory
static struct page *physical_page_ledger_ptr = NULL;
//delete?
static unsigned long ledger_size = 0;
static unsigned long nr_mem_pages = 0;

// Beginning of allocatable unused high memory kernel virtual addresses.
static uint8_t *unused_kernel_virt_addresses_ptr = NULL;

static uint32_t total_num_pte_per_pt = PAGE_SIZE/sizeof(pte_t);

// Reference kernel page directory
pde_t *ref_pgd;

static const int NUM_IDENTITY_PAGES = 1024;

extern void kernel_bug(void);

static int phys_to_ledger_idx(unsigned long);
uint8_t map_virt_to_phys(pde_t *, uint32_t, uint32_t, uint32_t);

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
    f_offset_into_idx = (phys_addr >> PAGE_SHIFT_SIZE)%PAGES_PER_BITMAP_UNIT;

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

// TODO: Derive a new address based on flags?
unsigned long derive_new_virt_addr(void)
{
    // allocate a free page
    unsigned long virt_addr = (unsigned long)unused_kernel_virt_addresses_ptr;

    // Set the next free page
    unused_kernel_virt_addresses_ptr += PAGE_SIZE;

    return virt_addr;
}

/*
 * Name:        pmm_get_free_frame
 *
 * Description: Allocate the available physical frame
 *
 * Arguments:   void
 *
 * Returns:     Success: Physical address of free frame
 *              Failure: 0
 *
 */
unsigned long pmm_get_free_frame()
{
    unsigned int i;

    for (i = 0; i < nr_mem_pages; i++)
    {
        if (physical_page_ledger_ptr[i].count == 0)
        {
            return i*PAGE_SIZE;
        }
    }

    return 0;
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
unsigned long alloc_page(unsigned long flags)
{
    unsigned long physical_addr = 0;
    unsigned long new_virt_addr = 0;
    int ledger_idx = 0;
    //int pd_idx = 0;
    //pde_t *kernel_pd = (pde_t *)KERNEL_PD_ADDR;

    physical_addr = pmm_get_free_frame();
    printk("%s: Found a free physical frame = %p\n", __func__, physical_addr);

    // TODO: If this fails, don't go any further.
    if (flags & GFP_KERNEL)
    {
        map_virt_to_phys((pde_t *)KERNEL_PD_ADDR, new_virt_addr, physical_addr, PAGE_PRESENT|PAGE_RW);
    }
   
    // Get the index into the ledger so we can mark as used
    ledger_idx = phys_to_ledger_idx(VIRT_TO_PHYS(new_virt_addr));

    // Mark that the page is being used.
    physical_page_ledger_ptr[ledger_idx].count++;
    
    printk("%s: Allocated page = %p\n", __func__, new_virt_addr);

    return new_virt_addr;
}

/*
 * Name:        boot_map_physical_page_ledger_ptr 
 *
 * Description: Create page table entries for the 'struct page' ledger array.
 *
 * Arguments: 	pgd_ptr - Page directory
 *
 * Return:      void	
 *
 * Virtual Address space:
 * 0xc0100000       0xc010#000          0xc010$000
 * |--------------------|-----------------------|--------------
 * | KERNEL             | phys page ledger      | Free memory
 * |--------------------|-----------------------|--------------
 * Physical Adress space:
 * 0x00100000       0x0010#000          0x0010$000
 *
 * The kernel's virtual address space is mapped to a backing physical address.
 * However, most of the physical page ledger has not been mapped yet.
 *
 * For each 'struct page' index of the physical page ledger, check if that
 * virtual address is mapped in a page table, if not map it. Create any page
 * tables that are necessary.
 *
 * 
 *
 */
void boot_map_physical_page_ledger_ptr(pde_t *pgd_ptr)
{
    int i;
int x = 0;
    uint32_t *pt_ptr = NULL;
    uint32_t *kernel_pd_ptr = (uint32_t *)pgd_ptr;
    struct page *page_ptr = physical_page_ledger_ptr;
    unsigned char *free_physical_memory_ptr = (unsigned char *)VIRT_TO_PHYS((unused_kernel_virt_addresses_ptr));
    unsigned long limit = (unsigned long)unused_kernel_virt_addresses_ptr;

    if (pgd_ptr == NULL)
    {
        printk("%s: pgd_ptr = %p\n", __func__, pgd_ptr);
        kernel_bug();
    }

    // TODO: Optimization: Don't need to check every index, check by page
    // frame!
    //
    // For each index in the frame ledger, check to see if that index is page
    // table mapped and accessible. If not, create the page table entry.
    while (((unsigned long)page_ptr) < limit)
    {
        i = get_pd_idx((uint32_t)page_ptr);

        // Check if there is a page table present that map the virtual address of
        // the current page. If not:
        // + Allocate a page but do not officially mark it yet. The page is
        //  "allocated" based on incrementing the free_physical_memory_ptr.
        // 
        if(is_pt_present(pgd_ptr, (unsigned long)page_ptr) != 1)
        {
            printk("%s: Page table missing for %p\n", __func__, (unsigned long)page_ptr);
       
            // Allocate a page table. 
            kernel_pd_ptr[i] = CREATE_PDE((uint32_t)free_physical_memory_ptr, PT_PRESENT|PT_RW);

            printk("is page table present? = %p\n", is_pt_present(pgd_ptr, (unsigned long)page_ptr));

            printk("kernel_pd_ptr [%p] >> %p\n", i, kernel_pd_ptr[i]);

            free_physical_memory_ptr += PAGE_SIZE;

            printk("B >> \n");
        }

//printk("%s kernel_pd_ptr[%p] = %p for virtual adress = %p\n", __func__, i, kernel_pd_ptr[i], page_ptr);

        if(is_pg_present((pte_t *)PAGE_SHIFT(pgd_ptr[i].pt_addr), (uint32_t)page_ptr) != 1)
        {
            pt_ptr = (uint32_t *)PAGE_SHIFT(pgd_ptr[i].pt_addr);

//            printk("%s: Page not mapped for %p\n", __func__, page_ptr);
            
            // 0. Get index of pte in a page table
            i = get_pt_idx((uint32_t)page_ptr);

//printk("PT index = %p\n", i);
//printk("%s pte = %p\n", __func__, pt_ptr[i]);
            // 1. Create a new pte for the next 4096 Bytes (bunch of struct
            //    pages)  in the ledger.
            pt_ptr[i] = CREATE_PTE((uint32_t)VIRT_TO_PHYS((uint32_t)page_ptr), PAGE_PRESENT | PAGE_RW);

//printk("%s pte = %p\n", __func__, pt_ptr[i]);
        }
//printk("%s pte = %p\n", __func__, ((pte_t *)PAGE_SHIFT((uint32_t)(pgd_ptr[i].pt_addr)))[i]);
//printk("%s: %p is mapped? = %p\n", __func__, page_ptr, is_page_mapped(pgd_ptr, (uint32_t)page_ptr));

        page_ptr += sizeof(struct page); 
x++;
    }

//printk("size of ledger = %p B (versus global = %p B) uses %p pages of ram\n", x*4, ledger_size, (x*4)/PAGE_SIZE);

    memset(physical_page_ledger_ptr, 0, ledger_size);

    // 
    // Mark every physical page that is used for the page ledger as in use.
    //
    page_ptr = physical_page_ledger_ptr;
x = 0;
    while ((unsigned long)page_ptr < (unsigned long)unused_kernel_virt_addresses_ptr)
    {
        // Clear the entire page

//printk("virt to phys = %p to %p\n", page_ptr, VIRT_TO_PHYS(page_ptr));
        i = phys_to_ledger_idx(VIRT_TO_PHYS((uint32_t)page_ptr));

        // The xth PAGE_SIZE page of the physical ledger (containing
        // PAGE_SIZE/sizeof(struct page) indices) is now officially in use!
        physical_page_ledger_ptr[i].count += 1;

        page_ptr += PAGE_SIZE;
x++;
    }

    // Finally, mark every new page table that we allocated in this function as
    // 'in use.' Obviously, this code won't execute if no new page tables were
    // allocated.
    unsigned long addr = (unsigned long)VIRT_TO_PHYS(unused_kernel_virt_addresses_ptr);
    while (addr < (unsigned long)free_physical_memory_ptr)
    {
        i = phys_to_ledger_idx((uint32_t)addr);
        physical_page_ledger_ptr[i].count += 1;
        addr += PAGE_SIZE;
    }

//printk("pd index = %p\n",get_pd_idx((uint32_t)page_ptr));
//while (1){}
}

/*
 *
 * This is run in protected mode before
 * paging is set up
 *
 */
void setup_memory(void)
{
    int i = 0;
    int total_nr_pages = 0;
    uint32_t kernel_end;
    uint32_t size_of_ledger = 0;
    uint32_t addr = 0;
    uint32_t identity_addr_ptr = 0;
    //pde_t *kernel_pd = (pde_t *)KERNEL_PD_ADDR;

	// Clear the frame bitmap
	//memset(frameLedger, 0, sizeof(frameLedger));

    // Organize and find out how much memory we have
    sanitize_meme820_map();

    total_nr_pages = get_total_nr_pages();
    if(total_nr_pages <= 0)
    {
        kernel_bug();
    }

	// Set the physical frame manager. 
    // TODO: Remove this debug information
    i = (int)get_pt_idx((uint32_t)0xc0100000);
    //printk(">> %p\n", i);
    uint32_t *pt_kernel = (uint32_t *)PT_KERNEL_ADDR; 
    while (pt_kernel[i] != 0){
        //printk(">>> %p is populated!\n", pt_kernel[i]);
        i++;
    }

    // TODO: Priority LOW
    // Add function in meme820 code that can query the memory map to verify
    // that there is in fact enough available memory for the memory map.

    // The starting address of the pages will be at the first page aligned
    // address after the kernel's end
    // This is a virtual address
    kernel_end = (uint32_t)(&__kernel_end);
    kernel_end += PAGE_SIZE;
    physical_page_ledger_ptr = (struct page *)PAGE_ALIGN(kernel_end);

    // Calclulate the size of the array of struct pages
    size_of_ledger = total_nr_pages*sizeof(struct page);
    
    // TODO: Remove?
    // These are global values used for testing
    ledger_size = size_of_ledger;
    nr_mem_pages = total_nr_pages;

    // Set pointer to the allocatable unused high memory kernel virtual addresses
    unused_kernel_virt_addresses_ptr =(uint8_t *)(physical_page_ledger_ptr + size_of_ledger);

    // If the unused high memory kernel virtual addresses does not align flush on a page boundary add
    // a page and align.
    //
    // if unused high memory kernel virtual addresses starts a bit into A, add a page then page align...
    // --------------           -------------------------
    // |            |           |           |           | 
    // |    A       |   ==>     |   A       |   B       |
    // |            |           |           |           |
    // --^-----------           ------------^--*---------
    //   |                                  |
    //
    // adding a page moves the marker to * , then PAGE_ALIGNing aligns to the
    // boundary between A and B.
    //
    if (((unsigned long)unused_kernel_virt_addresses_ptr%(unsigned long)PAGE_SIZE) != 0)
    {
        unused_kernel_virt_addresses_ptr += PAGE_SIZE;
        addr = (uint32_t)unused_kernel_virt_addresses_ptr;
        unused_kernel_virt_addresses_ptr = (uint8_t *)PAGE_ALIGN(addr);
    }

    boot_map_physical_page_ledger_ptr((pde_t *)KERNEL_PD_ADDR);

    //printk("Address of allocatable memory = %p\n", physical_page_ledger_ptr+size_of_ledger);
    //is_page_mapped(kernel_pd, 0x00400000 );
    //kernel_pd[get_pd_idx(physical_page_ledger+size_of_ledger)];
    //pt_kernel[x] = CREATE_PTE(, PAGE_PRESENT | PAGE_RW);
   
    // Offically reserve meme820 map
    reserve_meme820_pages();

    // Offically reserve the identity page table
    for (i = 0; i < NUM_IDENTITY_PAGES; identity_addr_ptr += PAGE_SIZE, i++)
    {
        // All pages should be mapped...
        if (is_page_mapped((pde_t *)KERNEL_PD_ADDR, identity_addr_ptr) == 1)
        {
            if (physical_page_ledger_ptr[i].count == 0)
            {
                physical_page_ledger_ptr[i].count += 1;
                //printk("%s: identity address = %p is in use = %p\n", __func__, identity_addr_ptr, physical_page_ledger_ptr[i].count);
            }
            continue;
        }
        else{
            printk("%s: Identity mapping does not exist for virtual address = %p\n", __func__, identity_addr_ptr);
            kernel_bug();
        }
    }
}

/*
 * Name:        phys_to_ledger_idx
 *
 * Description: Given a physical address find the index into the frame ledger.
 *
 * Arguments:   phys_addr
 *
 * Return:      Index of the physical address in the frame ledger
 *
 */
static int phys_to_ledger_idx(unsigned long phys_addr)
{
    return phys_addr >> 12;
}

/*
 * Name:        mark_page_used
 *
 * Description: Increment the use count the page that represents the
 *              corresponding physical frame.
 *
 * Arguments:   phys_addr - Physical address that should be marked in use.
 *
 * Return:      void
 *
 */
void mark_page_used(unsigned long phys_addr)
{
    int ledger_idx = phys_to_ledger_idx(phys_addr);

    //printk("%s: Marking page used = %p at ledger idx = %p\n", __func__, phys_addr, ledger_idx);

    physical_page_ledger_ptr[ledger_idx].count++;
}

/*
 * Name:        is_page_mapped
 *
 * Description: Given a page directory and virtual address
 *              return whether that virtual address is mapped.
 *
 * Arguments:   pd_ptr - page directory
 *              virt_addr
 *
 * Return:      1 - Yes
 *              0 - No
 *
 */
int is_page_mapped(pde_t *pd_ptr, uint32_t virt_addr)
{
    int i;

    if (pd_ptr == NULL){
        printk("%s: pd_ptr is NULL\n", __func__);
        kernel_bug();
    }   

    if(is_pt_present(pd_ptr, virt_addr) != 1)
    {
        printk("%s: Page table missing for %p\n", __func__, virt_addr);
        return 0;
    }

    i = get_pd_idx(virt_addr) ;

    // When referencing pt_addr, the value must be shifted
    // to get a real physical address. Physical address works
    // due to the identity mapping.
    if(is_pg_present((pte_t *)PAGE_SHIFT(pd_ptr[i].pt_addr),virt_addr) != 1)
    {
        printk("%s: Page not mapped for %p (pde_t = %p)\n", __func__, virt_addr, pd_ptr[i]);
        return 0;
    }

    //printk("Page mapping exists for virtual address = 0x%p\n", virt_addr);
    return 1;
}

/*
 * Name:        is_pt_present 
 *
 * Description: Given a page directory and virtual address, return whether
 *              there exists a page table (a valid pde_t) that maps that virtual
 *              address.
 *
 * Arguments:   pd_ptr - page directory
 *              virt_addr
 *
 * Return:      1 - Yes
 *              0 - No
 *
 */
int is_pt_present(pde_t *pd_ptr, uint32_t virt_addr)
{
    uint16_t pd_idx = 0;

    if (pd_ptr == NULL){
        printk("%s: pd_ptr is NULL\n", __func__);
        kernel_bug();
    }

    pd_idx = get_pd_idx(virt_addr);   

    //printk("%s: virt_addr = %p, pd_ptr[%p] = %p, %p\n", __func__, virt_addr, pd_idx, pd_ptr[pd_idx]);

    return (pd_ptr[pd_idx].present & PT_PRESENT); 
}

/*
 * Name:        is_pg_present
 *
 * Description:
 *
 * Arguments:
 *
 * Return:
 *
 */
int is_pg_present(pte_t *pt_ptr, uint32_t virt_addr)
{
    uint16_t pt_idx = 0;

    if (pt_ptr == NULL){
        printk("%s: pt_ptr is NULL\n", __func__);
        kernel_bug();
    }

    pt_idx = get_pt_idx(virt_addr);

    //printk("Page table = %p, index = %p, pte = %p\n", pt_ptr, pt_idx, pt_ptr[pt_idx]);

    return (pt_ptr[pt_idx].present & PT_PRESENT);
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
 * Name:        map_virt_to_phys
 *
 * Description: Create 
 *
 * Arguments:
 *
 * Returns:
 *
 */
uint8_t map_virt_to_phys(pde_t *pgd_ptr, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags)
{
    //uint32_t pd_idx;
    //uint32_t pt_idx;    
    uint32_t *pt_ptr = NULL;

    printk("pgd_ptr = %p, virt_addr = %p, phys_addr = %p, flags = %p\n", pgd_ptr, virt_addr, phys_addr, flags);

    if (pgd_ptr == NULL)
    {
        return 1;
    }

    if (is_pt_present(pgd_ptr, virt_addr) == 0)
    {
        
        // get a free frame from the physical page manager.

        printk("Allocating page for page directory!\n");

        // TODO: Remove the bug and handle this case
        //
        printk("UNHANDLED CASE!\n");
        kernel_bug();
    }
    
    // Check that the page we are to mark in memory is actually accounted for
    // itself
    // 
/*
    if (is_page_mapped(physical_page_ledger_ptr[ledger_idx]) == 0)
    {
        pd_idx = get_pd_idx(&physical_page_ledger_ptr[ledger_idx]);
        //VIRT_TO_PHYS(
        //pgd_ptr[pd_idx] = CREATE_PDE((uint32_t)pt_kernel, PT_PRESENT|PT_RW);
    }
    */

    // Get a pointer to the page table that has the virtual address
    // entry for the virtual address
    //
    // Casting isn't clean. pt_addr is 20 Bytes only. Casting in the following
    // manner allows the compiler warnings to go away
    pt_ptr = ((uint32_t *)(unsigned long)pgd_ptr[get_pd_idx(virt_addr)].pt_addr);

    // Create a page table entry (mapping of virtual address to physical
    // address) for a page table.
    printk(">> %p\n", pt_ptr[get_pt_idx(virt_addr)]);
    pt_ptr[get_pt_idx(virt_addr)] = CREATE_PTE(phys_addr, flags);
    printk(">> %p\n", pt_ptr[get_pt_idx(virt_addr)]);

    // TODO: Fix return
    return 0;    
}

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
    kernel_pd[x] = CREATE_PDE((uint32_t)pt_ident, PT_PRESENT|PT_RW);

	// Identy map alot of the kernel: (0x00000000 -> 0x00400000)
    // TODO: This can probably be reduced.
    x = 0;
	while ( x < NUM_IDENTITY_PAGES ){
        pt_ident[x] = CREATE_PTE(frameAddress, PAGE_PRESENT | PAGE_RW);

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
    kernel_pd[x] = CREATE_PDE((uint32_t)pt_kernel, PT_PRESENT|PT_RW); 

	x = get_pt_idx(kernel_start);
    virt_addr = kernel_start;
	frameAddress = physical_load_addr;
	while (virt_addr < kernel_end){
        pt_kernel[x] = CREATE_PTE(frameAddress, PAGE_PRESENT | PAGE_RW);

		frameAddress += PAGE_SIZE;
        virt_addr += PAGE_SIZE;
        x++;
	}
}

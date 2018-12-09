
/*
 * Where am I at?
 * In function boot_map_physical_page_ledger_ptr(), I added a while(1) to
 * investigate how many times the loop iterates. When I incrmement page_ptr
 * by PAGE_SIZE, I boot. I figured I want to increment by PAGE_SIZE b/c all
 * mappings I make will be at that cadence which would be less iterations than
 * if I incremented by sizeof(struct page); there are many sizeof(struct page)
 * increments in a single PAGE_SIZE
 *
 *
 * Problems:
 * 0. The page directory/table traversal function is using physical addresses
 *    like virtual addresses. I may get away with this now, but I won't be
 *    able to in the future. James Molloy has a strategy for traversing the
 *    page tables.
 *
 * 1. How do I track virtual addresses?
 *
 *
 */

#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/meme820.h"
#include "x86/inc/arch_mm.h"

// Array of struct pages that describe each page of physical memory
static struct page *physical_page_ledger_ptr = NULL;
//delete?
static unsigned long ledger_size = 0;
static unsigned long nr_mem_pages = 0;

// Beginning of allocatable unused high memory kernel virtual addresses.
static uint8_t *unused_kernel_virt_addresses_ptr = NULL;

static uint32_t total_num_pte_per_pt = PAGE_SIZE/sizeof(pte_t);

static const int NUM_IDENTITY_PAGES = 1024;

extern void kernel_bug(void);

static int phys_to_ledger_idx(unsigned long);
uint8_t map_virt_to_phys(pde_t *, uint32_t, uint32_t, uint32_t);

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

    // When referencing pt_phys_addr, the value must be shifted
    // to get a real physical address. Physical address works
    // due to the identity mapping.
    if(is_pg_present((pte_t *)PAGE_SHIFT(pd_ptr[i].pt_phys_addr),virt_addr) != 1)
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
    // Casting isn't clean. pt_phys_addr is 20 Bytes only. Casting in the following
    // manner allows the compiler warnings to go away
    pt_ptr = ((uint32_t *)(unsigned long)pgd_ptr[get_pd_idx(virt_addr)].pt_phys_addr);

    // Create a page table entry (mapping of virtual address to physical
    // address) for a page table.
    printk(">> %p\n", pt_ptr[get_pt_idx(virt_addr)]);
    pt_ptr[get_pt_idx(virt_addr)] = CREATE_PTE(phys_addr, flags);
    printk(">> %p\n", pt_ptr[get_pt_idx(virt_addr)]);

    // TODO: Fix return
    return 0;    
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
    uint32_t *pt_ptr = NULL;
    uint32_t *kernel_pd_ptr = (uint32_t *)pgd_ptr;
    struct page *page_ptr = NULL;
    unsigned char *free_physical_memory_ptr = (unsigned char *)VIRT_TO_PHYS((unused_kernel_virt_addresses_ptr));
    unsigned long limit = 0;

    if (pgd_ptr == NULL)
    {
        printk("%s: pgd_ptr = %p\n", __func__, pgd_ptr);
        kernel_bug();
    }

    // For each index in the frame ledger, check to see if that index is page
    // table mapped and accessible. If not, create the page table entry.
    page_ptr = physical_page_ledger_ptr;
    limit = (unsigned long)unused_kernel_virt_addresses_ptr;
    while (((unsigned long)page_ptr) < limit)
    {
        i = get_pd_idx((uint32_t)page_ptr);

        //
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
        }

        // If the virtual address does not have a mapping in the page table,
        // make a mapping.
        if(is_pg_present((pte_t *)PAGE_SHIFT(pgd_ptr[i].pt_phys_addr), (uint32_t)page_ptr) != 1)
        {
            pt_ptr = (uint32_t *)PAGE_SHIFT(pgd_ptr[i].pt_phys_addr);

            // 0. Get index of pte in a page table
            i = get_pt_idx((uint32_t)page_ptr);

            // 1. Create a new pte for the next 4096 Bytes (bunch of struct
            //    pages)  in the ledger.
            pt_ptr[i] = CREATE_PTE((uint32_t)VIRT_TO_PHYS((uint32_t)page_ptr), PAGE_PRESENT | PAGE_RW);

//printk("Created a mapping for %p, i = %p\n", page_ptr, i);
        }
   
        // Increment the page_ptr by 1x full page 
        page_ptr = (struct page *)((unsigned char *)(page_ptr) + PAGE_SIZE); 
    }

    // Clear the entire ledger for the physical pages.
    memset(physical_page_ledger_ptr, 0, ledger_size);

    // 
    // Mark every physical page that is used for the page ledger 'in use'
    //
    page_ptr = physical_page_ledger_ptr;
    while ((unsigned long)page_ptr < (unsigned long)unused_kernel_virt_addresses_ptr)
    {
        i = phys_to_ledger_idx(VIRT_TO_PHYS((uint32_t)page_ptr));

        // The xth PAGE_SIZE page of the physical ledger (containing
        // PAGE_SIZE/sizeof(struct page) indices) is now officially in use!
        physical_page_ledger_ptr[i].count += 1;

        page_ptr += PAGE_SIZE;
    }

    //
    // Finally, mark every new page table that we allocated in this function as
    // 'in use,' if we allocated any.
    // //
    unsigned long addr = (unsigned long)VIRT_TO_PHYS(unused_kernel_virt_addresses_ptr);
    while (addr < (unsigned long)free_physical_memory_ptr)
    {
        i = phys_to_ledger_idx((uint32_t)addr);
        physical_page_ledger_ptr[i].count += 1;
        addr += PAGE_SIZE;
    }
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

    // Organize and find out how much memory we have
    sanitize_meme820_map();

    total_nr_pages = get_total_nr_pages();
    if(total_nr_pages <= 0)
    {
        kernel_bug();
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

    //
    // Set a marker to unused kernel virtual address.
    // This is the first page aligned addresses after the frame ledger ends.
    //
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
        // If I try to shorten this, the PAGE_ALIGN macro freaks out. Turn it
        // into a funciton?
        unused_kernel_virt_addresses_ptr += PAGE_SIZE;
        addr = (uint32_t)unused_kernel_virt_addresses_ptr;
        unused_kernel_virt_addresses_ptr = (uint8_t *)PAGE_ALIGN(addr);
    }

    // 1. Map virtual addresses to frame ledger (create page tables as
    //    necessary)
    // 2. Memset the frame ledger.
    // 3. Within the frame ledger, mark memory used by the frame ledger in use.
    // 4. Mark any allocated space for page tables as in use
    boot_map_physical_page_ledger_ptr((pde_t *)KERNEL_PD_ADDR);

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
            else
            {
                printk("%s: Identity page with virtual/physical address = %p, alread in use.\n", __func__, identity_addr_ptr);
                kernel_bug();
            }
        }
        else{
            printk("%s: Identity mapping does not exist for virtual address = %p\n", __func__, identity_addr_ptr);
            kernel_bug();
        }
    }
}

// This function is called from low memory identity mapped code
// to help get into paging mode successfully. 
/* Name:        setupPaging
 *
 * Abstract:    Initializes page tables used for booting.
 *
 * Description: This function is called during boot when the process is in real
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

    //
}

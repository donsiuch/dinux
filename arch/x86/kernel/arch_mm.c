
/*
 * Name: arch_mm.c 
 * 
 * This file contains:
 *  - Paging initialization
 *  - Evalation of meme820 results
 *  - Initialization of memory manager 
 *  - Physical memory manager
 *  - Memory statistics ( TODO: move to generic mm.c file?)
 *  - Page directory traversals + manipulations
 *
 */

#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/meme820.h"
#include "x86/inc/arch_mm.h"
#include "dinux/inc/vmm.h"

#define NUM_IDENTITY_PAGES 1024

static uint32_t total_num_pte_per_pt = PAGE_SIZE/sizeof(pte_t);

// Statistics about memory usage
struct memory_stats mem_stats;

// Array of struct pages that describe each page of physical memory
struct page *physical_page_ledger = NULL;

// Function prototypes
static int phys_to_ledger_idx(unsigned long);

extern unsigned long unused_virt_addr_ptr;

extern int power(int, int);
extern void kernel_bug(void);

/*
 * Name:        pmm_get_free_frame
 *
 * Description: Allocate an available physical frame
 *
 * Arguments:   void
 *
 * Returns:     Success: Physical address of free frame
 *              Failure: 0
 *
 */
unsigned long pmm_get_free_frame(void)
{
    unsigned int i;
    unsigned long nr_phys_frames = mem_stats.nr_total_frames;

    for (i = 0; i < nr_phys_frames; i++)
    {
        if (physical_page_ledger[i].count == 0)
        {
            return i*PAGE_SIZE;
        }
    }

    return 0;
}

/*
 * Name:        pmm_mark_frame_in_use
 *
 * Description: Increment the use count the page that represents the
 *              corresponding physical frame.
 *
 * Arguments:   phys_addr - Physical address that should be marked in use.
 *
 * Return:      void
 *
 */
void pmm_mark_frame_in_use(unsigned long phys_addr)
{
    int ledger_idx = phys_to_ledger_idx(phys_addr);

    //printk("%s: Marking page used = %p at ledger idx = %p\n", __func__, phys_addr, ledger_idx);

    physical_page_ledger[ledger_idx].count++;

    // Update memory statistics
    mem_stats.nr_used_frames++;
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
    return (phys_addr >> 12);
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
 * Name: is_page_present
 *
 * Description: Return whether a physical frame is mapped to a address.
 *
 * Arguments:   virt_addr 
 *
 * Returns:     1 - Physical frame is present
 *              0 - Physical frame is not present.
 *
 */
int is_page_present(unsigned virt_addr)
{
    pte_t *pte_ptr = NULL;

    pte_ptr = (pte_t *)(SELF_MAP_ADDR 
                + get_pd_idx(virt_addr)*PAGE_SIZE 
                    + get_pt_idx(virt_addr)*sizeof(pte_t));

    if (pte_ptr->present & PAGE_PRESENT)
    {
        //printk("pte_ptr = *%p = %p\n", pte_ptr, *(uint32_t *)pte_ptr);
        return 1;
    }
    
    //printk("%s: Page is missing. *%p = %p\n", __func__, pte_ptr, *(uint32_t *)pte_ptr);

    return 0;
}

/*
 * Name: is_pt_present
 *
 * Description: Return whether a page table is present for a virtual address.
 *
 * Arguments:   virt_addr
 *
 * Returns:     1 - Page table is present
 *              0 - Page table is not present
 *
 */
int is_pt_present(unsigned long virt_addr)
{
    pde_t *pde_ptr = NULL;

    pde_ptr = (pde_t *)(SELF_MAP_ADDR 
                + get_pd_idx(SELF_MAP_ADDR)*PAGE_SIZE 
                    + get_pd_idx(virt_addr)*sizeof(pde_t));

    if (pde_ptr->present & PT_PRESENT)
    {
        //printk("%s: Page table present (%p). pde_ptr = *%p = %p\n", __func__, 
        //  virt_addr,  pde_ptr, *(uint32_t *)pde_ptr);
        return 1;
    }

    //printk("%s: Page table missing. *%p = %p\n", __func__, pde_ptr, *pde_ptr);

    return 0;
}

/*
 * Name: invalidate_tlb_entry
 *
 * Description: Invalidate TLB entries
 *
 * Arguments: virt_addr
 *
 * Returns: void
 *
 * Note: Apparently the page must be *accessed* to invalidate it.
 *			(I don't see this in the Intel doc describing the function)
 * 		!!!BE CAREFUL when freeing a page!!! 
 *
 */
void invalidate_tlb_entry(unsigned long virt_addr)
{
	__asm__ volatile ("invlpg (%%eax);" :: "a"(virt_addr) );
}

/*
 * Name: install_page
 *
 * Description: Given a virtual address and the address of a physical frame
 *              map that virtual address to that physical frame (creates
 *              a pte_t and installs in page table).
 *
 * Arguments:   virt_addr
 *              phys_addr - Address of physical frame page
 *
 * Returns:     void
 * 
 * Note: This function does not check for an installed page table (aka. pde_t)
 * for that virtual address.
 *
 */
void install_page(unsigned long virt_addr, unsigned long phys_addr)
{
    pte_t *pte_ptr = NULL;

    pte_ptr = (pte_t *)(SELF_MAP_ADDR 
                + get_pd_idx(virt_addr)*PAGE_SIZE 
                    + get_pt_idx(virt_addr)*sizeof(pte_t));
    
    *(uint32_t *)pte_ptr = CREATE_PTE(phys_addr, PAGE_PRESENT|PAGE_RW);

	invalidate_tlb_entry(virt_addr);

    memset((void *)virt_addr, 0, PAGE_SIZE);
}

/*
 * Name: install_page_table
 *
 * Description: Install a physical frame as the page table for the a virtual 
 *              address. (creates a pde_t and set in the page directory corresponding to
 *              that virtual address.
 *
 *  Arguments:  virt_addr
 *              phy_addr - Address of physical frame page table
 *
 *  Returns:    void
 *
 */
void install_page_table(unsigned long virt_addr, unsigned long phys_addr)
{
    pde_t *pde_ptr = NULL;
    unsigned long addr = 0;

    pde_ptr = (pde_t *)(SELF_MAP_ADDR 
                + get_pd_idx(SELF_MAP_ADDR)*PAGE_SIZE 
                    + get_pd_idx(virt_addr)*sizeof(pde_t));

    *(uint32_t *)pde_ptr = CREATE_PDE((uint32_t)phys_addr, PT_PRESENT|PT_RW);

	invalidate_tlb_entry(virt_addr);

    // Calculate the address that will resolve to the top of
    // newly installed page table
    addr = SELF_MAP_ADDR 
                + get_pd_idx(virt_addr)*PAGE_SIZE 
                    + get_pt_idx(0)*sizeof(pte_t);

    // Clear the newly installed page table
    memset((void *)addr, 0, PAGE_SIZE);

    //printk("%s: Installed page table for %p\n", __func__, virt_addr);
}

/*
 * Name: map_virt_to_phys 
 *
 * Description: Map a virtual address to the physical address of a physical frame.
 *              This function may allocate and install a page table if one is
 *              missing for given virtual address.
 *
 * Arguments:   virt_addr
 *              phys_addr - Address of physical frame
 *
 * Returns:     void
 *
 */
void map_virt_to_phys(unsigned long virt_addr, unsigned long phys_addr)
{   
    unsigned long new_addr = 0;

    // Check if a page table is present.
    // If not, get frame and install it. 
    if (is_pt_present(virt_addr) == 0)
    {
        printk("Page table isn't present for virtual address = %p\n", virt_addr);
        new_addr = pmm_get_free_frame();

        if (new_addr == 0)
        {
            printk("%s: Could not get free frame.\n");
            kernel_bug(); 
        }

        pmm_mark_frame_in_use(new_addr);

        install_page_table(virt_addr, new_addr);
    }

    if (is_page_present(virt_addr) != 0)
    {
        printk("%s: Virtual address already has a mapping.\n");
        kernel_bug();
    }

    install_page(virt_addr, phys_addr);
}

#if 0
void unmap_virt(unsigned long virt)
{

}
#endif

/*
 * Name:        boot_map_physical_page_ledger 
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
 */
void boot_map_physical_page_ledger()
{
    struct page *page_ptr = NULL;
    unsigned char *free_physical_memory_ptr = (unsigned char *)VIRT_TO_PHYS((unused_virt_addr_ptr));
    unsigned long limit = 0;

    // For each index in the frame ledger, check to see if that index is page
    // table mapped and accessible. If not, create the page table entry.
    page_ptr = physical_page_ledger;
    limit = (unsigned long)unused_virt_addr_ptr;
    while (((unsigned long)page_ptr) < limit)
    {
        //
        // Check if there is a page table present that map the virtual address of
        // the current page. If not:
        // + Allocate a page but do not officially mark it yet. The page is
        //  "allocated" based on incrementing the free_physical_memory_ptr.
        // 
        if(is_pt_present((unsigned long)page_ptr) != 1)
        {
            install_page_table((unsigned long)page_ptr, (unsigned long)free_physical_memory_ptr);

            free_physical_memory_ptr += PAGE_SIZE;
        }

        // If the virtual address does not have a mapping in the page table,
        // make a mapping.
        if(is_page_present((unsigned long)page_ptr) != 1)
        {
            install_page((unsigned long)page_ptr, VIRT_TO_PHYS((uint32_t)page_ptr));

            //printk("Created a mapping for %p, i = %p\n", page_ptr, i);
        }
   
        // Increment the page_ptr by 1x full page 
        page_ptr = (struct page *)((unsigned char *)(page_ptr) + PAGE_SIZE); 
    }

    // Clear the entire ledger for the physical pages.
    memset(physical_page_ledger, 0, mem_stats.nr_total_frames*sizeof(struct page));

    // 
    // Now that the virtual addresses of the physical_page_ledger are
    // installed into the page table, mark them in use.
    // 
    // Mark every physical page that is used for the page ledger 'in use'
    //
    page_ptr = physical_page_ledger;
    while ((unsigned long)page_ptr < (unsigned long)unused_virt_addr_ptr)
    {
        // The xth PAGE_SIZE page of the physical ledger (containing
        // PAGE_SIZE/sizeof(struct page) indices) is now officially in use!
        pmm_mark_frame_in_use(VIRT_TO_PHYS((uint32_t)page_ptr));

        page_ptr += PAGE_SIZE;
    }

    //
    // Finally, mark every new page table that we allocated in this function as
    // 'in use,' if we allocated any.
    // //
    unsigned long addr = (unsigned long)VIRT_TO_PHYS(unused_virt_addr_ptr);
    while (addr < (unsigned long)free_physical_memory_ptr)
    {
        pmm_mark_frame_in_use(addr);
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
    uint32_t kernel_end;
    uint32_t size_of_ledger = 0;
    uint32_t identity_addr_ptr = 0;

    memset(&mem_stats, 0, sizeof(mem_stats));

    // Organize and find out how much memory we have
    sanitize_meme820_map();

    mem_stats.nr_total_frames = get_total_nr_pages();
    
    if(mem_stats.nr_total_frames <= 0)
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
    physical_page_ledger = (struct page *)PAGE_ALIGN(kernel_end);

    // Calclulate the size of the array of struct pages
    size_of_ledger = mem_stats.nr_total_frames*sizeof(struct page);
    
    // Set pointer to the first unused virtual memory address after the frame ledger. 
    unused_virt_addr_ptr = (unsigned long)physical_page_ledger + size_of_ledger;

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
    if (((unsigned long)unused_virt_addr_ptr%(unsigned long)PAGE_SIZE) != 0)
    {
        // No more than PAGE_SIZE-1 Bytes of space will be unused in the worst
        // case 
        unused_virt_addr_ptr += PAGE_SIZE;
        unused_virt_addr_ptr = PAGE_ALIGN(unused_virt_addr_ptr);
    }
    
    // 1. Map virtual addresses to frame ledger (create page tables as
    //    necessary)
    // 2. Memset the frame ledger.
    // 3. Within the frame ledger, mark memory used by the frame ledger in use.
    // 4. Mark any allocated space for page tables as in use
    boot_map_physical_page_ledger();

    // Offically reserve meme820 map
    reserve_meme820_pages();

    //
    // Offically reserve the identity mapped pages
    //
    for (i = 0; i < NUM_IDENTITY_PAGES; identity_addr_ptr += PAGE_SIZE, i++)
    {
        // There may be meme820 'used' regions within the identity region.
        // Allow those 'used' regions to increment to 2.
        pmm_mark_frame_in_use(identity_addr_ptr);
    }
}

/* Name:        setupPaging
 *
 * Abstract:    Initializes page tables used for booting.
 *
 * Description: This function is called during boot when the process is in real
 *              mode. The purpose is to initialize the page tables for to prep for 
 *              the switch to protected mode.
 *              
 *              This function has three main parts:
 *                  Identity map 0x00000000 - 0x00400000
 *                      - This region contains the boot pd and pt
 *                      - Real mode code
 *                      - Terminal buffer
 *                      - BIOS code (must be accessed from real mode though...)
 *
 *                  Map __kernel_start virtual address to where it is actually
 *                  loaded in memory.
 *
 *                  Map the last entry of the page directory to itself.
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
    // PART 3
    // Create the self page mapping
    //
    kernel_pd[NUM_PD_ENTRIES - 1] = CREATE_PDE((uint32_t)kernel_pd, PAGE_PRESENT | PAGE_RW);
}

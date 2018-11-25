
#include "dinux/inc/io.h"
#include "x86/inc/arch_mm.h"
#include "x86/inc/meme820.h"

extern unsigned long num_avail_pages;

extern void kernel_bug(void);

#if 0
/*
 * This is a test function.
 */
void build_map()
{
    struct meme820 *meme820_raw_ptr = (struct meme820 *)MEME820_ADDR;

#if 0
    // The two regions are merge together.
    // Followed by magic stop
    meme820_raw_ptr[0].base_addr_low = 0;
    meme820_raw_ptr[0].length_low = 4096;
    meme820_raw_ptr[0].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[1].base_addr_low = 4096;
    meme820_raw_ptr[1].length_low = 4096;
    meme820_raw_ptr[1].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[2].base_addr_low = MEME820_MAGIC_STOP;
#endif

#if 0
    // The two regions are merged together.
    // Followed by reserved than a stop.
    meme820_raw_ptr[0].base_addr_low = 0;
    meme820_raw_ptr[0].length_low = 4096;
    meme820_raw_ptr[0].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[1].base_addr_low = 4096;
    meme820_raw_ptr[1].length_low = 4096;
    meme820_raw_ptr[1].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[2].base_addr_low = 8192;
    meme820_raw_ptr[2].length_low = 777;
    meme820_raw_ptr[2].type = ADDR_RANGE_RESERVED;
    meme820_raw_ptr[3].base_addr_low = MEME820_MAGIC_STOP;
#endif

#if 0
    // 6 regions: 
    // 0-1: Available merges
    // 2: Reserved
    // 3-4: Available merges
    // 5: Stop
    meme820_raw_ptr[0].base_addr_low = 0;
    meme820_raw_ptr[0].length_low = 4096;
    meme820_raw_ptr[0].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[1].base_addr_low = 4096;
    meme820_raw_ptr[1].length_low = 4096;
    meme820_raw_ptr[1].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[2].base_addr_low = 8192;
    meme820_raw_ptr[2].length_low = 4096;
    meme820_raw_ptr[2].type = ADDR_RANGE_RESERVED;

    meme820_raw_ptr[3].base_addr_low = 12288;
    meme820_raw_ptr[3].length_low = 4096;
    meme820_raw_ptr[3].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[4].base_addr_low = 16384;
    meme820_raw_ptr[4].length_low = 4096;
    meme820_raw_ptr[4].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[5].base_addr_low = MEME820_MAGIC_STOP;
#endif

#if 0
    // 0-1: available regions
    // 2: too small available region
    // 3: Stop 
    meme820_raw_ptr[0].base_addr_low = 0;
    meme820_raw_ptr[0].length_low = 4096;
    meme820_raw_ptr[0].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[1].base_addr_low = 4096;
    meme820_raw_ptr[1].length_low = 4096;
    meme820_raw_ptr[1].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[2].base_addr_low = 8192;
    meme820_raw_ptr[2].length_low = 1012;
    meme820_raw_ptr[2].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[3].base_addr_low = MEME820_MAGIC_STOP; 
#endif

#if 0
    // 0-1: Available
    // 2: Avialable too large
    // 3: Stop
    //
    meme820_raw_ptr[0].base_addr_low = 0;
    meme820_raw_ptr[0].length_low = 8192;
    meme820_raw_ptr[0].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[1].base_addr_low = 8192;
    meme820_raw_ptr[1].length_low = 4096;
    meme820_raw_ptr[1].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[2].base_addr_low = 12288;
    meme820_raw_ptr[2].length_low = 10000;
    meme820_raw_ptr[2].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[3].base_addr_low = MEME820_MAGIC_STOP;
#endif

    // 0-1: Available
    // 2: Reserved
    // 3: Avialable too large
    // 4: Stop
    //
    meme820_raw_ptr[0].base_addr_low = 0;
    meme820_raw_ptr[0].length_low = 8192;
    meme820_raw_ptr[0].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[1].base_addr_low = 8192;
    meme820_raw_ptr[1].length_low = 4096;
    meme820_raw_ptr[1].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[2].base_addr_low = 12288;
    meme820_raw_ptr[2].length_low = 4096;
    meme820_raw_ptr[2].type = ADDR_RANGE_RESERVED;
    meme820_raw_ptr[3].base_addr_low = 16384;
    meme820_raw_ptr[3].length_low = 10000;
    meme820_raw_ptr[3].type = ADDR_RANGE_MEMORY;
    meme820_raw_ptr[4].base_addr_low = MEME820_MAGIC_STOP;
}
#endif

/* Name:        dump_raw_region 
 *
 * Description: Prints a meme820 region. 
 *
 * Arguments:   raw_ptr - Pointer to a meme820 entry.
 *
 * Returns:     void 
 *
 */
static void dump_raw_region(struct meme820 *raw_ptr)
{
    if (!raw_ptr)
            kernel_bug();

    if (raw_ptr->base_addr_low == MEME820_MAGIC_STOP)
    {
        printk("End of region\n");
        return;
    }


    printk("0x%p%p 0x%p%p %p ", raw_ptr->base_addr_high, raw_ptr->base_addr_low, raw_ptr->length_high, raw_ptr->length_low, raw_ptr->type);

    if (raw_ptr->type == ADDR_RANGE_MEMORY)
        printk("[ Available ]\n");
    else
        printk("[ Reserved ]\n");

}

void dump_all_regions(struct meme820 *raw_ptr)
{
    int i;

    printk("E820 Full Map:\n");

    for (i = 0; i < MEME820_MAX_NR && raw_ptr[i].base_addr_low != MEME820_MAGIC_STOP ; i++)
    {
        dump_raw_region(&raw_ptr[i]);
    }
}

/*
 * Name:        reserve_meme820_pages 
 *
 * Description: Loop through the meme820 sanitized map and mark pages reserved. 
 *              This function is called once when memory is being setup.
 *
 * Arguments:   physical_page_ledger_ptr 
 *
 * Returns:     void
 *
 */
void reserve_meme820_pages()
{
    int i;
    unsigned long limit_addr = 0;
    unsigned long raw_page_ptr = 0x00;

    struct meme820 *meme820_map_ptr = (struct meme820 *)MEME820_ADDR;

    for (i = 0; i < MEME820_MAX_NR; i++)
    {
        // Check stopping condition
        if (meme820_map_ptr[i].base_addr_low == MEME820_MAGIC_STOP)
        {
            break;
        }

        // If the memory region is available, go to the next region.
        if (meme820_map_ptr[i].type == ADDR_RANGE_MEMORY)
        {
            continue;
        }
  
        //
        // Evaluating a used region!
        //

        // Get the starting address
        raw_page_ptr = meme820_map_ptr[i].base_addr_low; 
      
        // Find the address of the first page outside of the region (the
        // stopping condition. 
        limit_addr = (unsigned long)raw_page_ptr + meme820_map_ptr[i].length_low;

        // Starting at the second region, try to detect gaps
        // to mark as used.
        if (i > 0)
        {
            // If the start of the region currently being evaluated is not
            // equal to the end address of the last region, we detected a gap.
            //
            // Since the current region is "used," and the previous immediate
            // x numver of pages are a gap (also "used"), set the starting
            // point to that earlier address.
            if (raw_page_ptr > meme820_map_ptr[i-1].base_addr_low + meme820_map_ptr[i-1].length_low)
            {
                //printk("%s: Detected gap! = %p, %p, %p\n", __func__, raw_page_ptr, meme820_map_ptr[i-1].base_addr_low + meme820_map_ptr[i-1].length_low, i);
                raw_page_ptr =  meme820_map_ptr[i-1].base_addr_low + meme820_map_ptr[i-1].length_low;
            }
        }

        //printk("%s: Mapping range = %p to %p\n", __func__, raw_page_ptr, limit_addr );

        // While we are evaluating a page within the region
        while (raw_page_ptr < limit_addr)
        {
            //printk("Marking = %p as used\n", raw_page_ptr);
            mark_page_used(raw_page_ptr); 
            raw_page_ptr += PAGE_SIZE;
        }
    }

}

/*
 * Name:        get_total_nr_pages 
 *
 * Description: Loop through the sanitized meme820 memory map and get
 *              the total number of pages.
 *
 * Arguments:   void
 *
 * Returns:     Total number of memory pages 
 *
 */
int get_total_nr_pages(void)
{
    int i;
    int nr_pages_before_last_entry = 0;
    int nr_last_entry_pages = 0;

    struct meme820 *meme820_map_ptr = (struct meme820 *)MEME820_ADDR;
   
    // Locate the last entry. 
    for (i = 0; i < MEME820_MAX_NR ; i++)
    {
        // Check stopping condition
        if (meme820_map_ptr[i].base_addr_low == MEME820_MAGIC_STOP)
        {
            break;
        }
    }

    // Check if the first entry is the stopping condition
    if (i == 0)
    {
        return 0;
    }

    // Return the number of pages 
    nr_pages_before_last_entry = meme820_map_ptr[i-1].base_addr_low/PAGE_SIZE;
    nr_last_entry_pages = meme820_map_ptr[i-1].length_low/PAGE_SIZE; 

    //printk("Size of memory = %p\n", nr_pages_before_last_entry + nr_last_entry_pages);

    return nr_pages_before_last_entry + nr_last_entry_pages; 
}

/* Name: sanitize_meme820_map 
 *
 * Description: Scan the meme820 results and store in kernel data. 
 *
 * Arguments:  void
 *
 * Returns:    void 
 *
 * Assumptions:
 * - There are MEME820_MAX_NR or less e820 map entries. 
 * - There are no overlapping regions
 * - No values are more than 4 Bytes-- 32 bits
 *
 */
void sanitize_meme820_map(void)
{
    int i, j, trimmed_bytes;
    struct meme820 *meme820_raw_ptr = (struct meme820 *)MEME820_ADDR;

    //dump_all_regions(meme820_raw_ptr);

    //printk("E820 Map:\n");

    // i lags behind an points to the region we are merging
    // j looks ahead for the first unavailable region
    for (i = 0, j = 0; i < MEME820_MAX_NR ; i = j )
    {
        //printk("Region:\n");
        //dump_raw_region(&(meme820_raw_ptr[i]));

        // Check stopping condition
        if (meme820_raw_ptr[i].base_addr_low == MEME820_MAGIC_STOP)
        {
            break;
        }

        // if the current region is reserved, go to the next region. 
        if ( meme820_raw_ptr[i].type == ADDR_RANGE_RESERVED )
        {
            // since the loop condition brings i forward to j,
            // just increase j here
            j++;

            continue;
        }
        
        // We have found an available region.
        // Merge all adjacent available regions. 
        //
        // Recall i = j at this point, so have j start scanning forward at the
        // next region.
        for ( j = i+1 ; j < MEME820_MAX_NR && meme820_raw_ptr[j].type == ADDR_RANGE_MEMORY ; j++ )
        {
            // If j reaches the end break out of the merging step
            if ( meme820_raw_ptr[j].base_addr_low == MEME820_MAGIC_STOP )
            {
                break;
            }
            
            //printk("Merging adjacent available region:\n");
            //dump_raw_region(&(meme820_raw_ptr[j]));
            
            // Currently only supports 32 bit regions.
            // Not worrying about the carry to 33rd bit.
            //
            // Add the size of the next available region to this region.
            meme820_raw_ptr[i].length_low += meme820_raw_ptr[j].length_low; 

            // Make the address range reserved so it
            // can be ignored.
            meme820_raw_ptr[j].type = ADDR_RANGE_RESERVED;
        }

        // At this point, i resides on a memory region. This region can be:
        // a. Smaller than PAGE_SIZE (mark unavailable).
        // b. Larger than a page size. Trim off Bytes beyond the page to make
        //  it a multiple of a PAGE_SIZE.
        // c. A multiple of PAGE_SIZE (we are done normalizing this region)

        // a. Too small, mark as unavailable
        if (meme820_raw_ptr[i].length_low < PAGE_SIZE)
        {
            meme820_raw_ptr[i].type = ADDR_RANGE_RESERVED; 
        }

        // b. Larger than a page size. Trim off Bytes beyond the page to make
        //  it a multiple of a PAGE_SIZE.
        if (meme820_raw_ptr[i].length_low%PAGE_SIZE != 0)
        {
            trimmed_bytes = meme820_raw_ptr[i].length_low - PAGE_SIZE*(meme820_raw_ptr[i].length_low/PAGE_SIZE);
            meme820_raw_ptr[i].length_low = PAGE_SIZE*(meme820_raw_ptr[i].length_low/PAGE_SIZE);

            // As long as the j region is not the end; it is on a reserved
            // region.
            // Subtract the start of the next region to account for the
            // trimming above.
            //
            // This is not necessary because this is fixing up a reserved
            // region which is currently unnacounted for. Nice for visual
            // purposes.
            if(meme820_raw_ptr[j].base_addr_low != MEME820_MAGIC_STOP)
            {
                meme820_raw_ptr[j].base_addr_low -= trimmed_bytes;
                meme820_raw_ptr[j].length_low += trimmed_bytes;
            }
        }

        // Region has been normalized.

        num_avail_pages += (meme820_raw_ptr[i].length_low/PAGE_SIZE);

        //printk("Fixed up region:\n");
        //dump_raw_region(&(meme820_raw_ptr[i]));
    }

    //dump_all_regions(meme820_raw_ptr);

    //printk("Number of available pages: 0x%p", num_avail_pages);
}


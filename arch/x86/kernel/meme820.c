
#include "dinux/inc/io.h"
#include "x86/inc/meme820.h"
#include "x86/inc/arch_mm.h"

extern unsigned long num_avail_pages;
extern unsigned long num_used_pages;

extern void kernel_bug(void);

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

static unsigned long normalize_region(struct meme820 *mem_ptr, int i)
{
    dump_raw_region(mem_ptr);

    return 0;
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
    int i, j;
    struct meme820 *meme820_raw_ptr = (struct meme820 *)MEME820_ADDR;

    printk("E820 Map:\n");

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
            meme820_raw_ptr[i].length_low = PAGE_SIZE*(meme820_raw_ptr[i].length_low/PAGE_SIZE);
        }

        // Region has been normalized.

        //printk("[ Available ]\n");
        num_avail_pages += (meme820_raw_ptr[i].length_low/PAGE_SIZE);

        //printk("Fixed up region:\n");
        //dump_raw_region(&(meme820_raw_ptr[i]));
    }
}


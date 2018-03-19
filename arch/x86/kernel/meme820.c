
#include "dinux/inc/io.h"
#include "x86/inc/meme820.h"
#include "x86/inc/arch_mm.h"

/* Name: sanitize_meme820_map 
 *
 * Description: Scan the meme820 results and store in kernel data. 
 *
 * Arguments:  void
 *
 * Returns:    void 
 *
 * Assumes there are 1024 or less e820 map entries. 
 *
 */
void sanitize_meme820_map(void)
{
    int i;
    int offset = 0;
    int upper_limit = 1024;
    struct meme820 raw;

    printk("E820 Map:\n");

    for (i = 0; i < upper_limit; i++, offset += MEME820_RESULT_SIZE)
    {
        raw = *(struct meme820 *)(MEME820_ADDR + offset);

        // Check stopping condition
        if (raw.base_addr_low == MEME820_MAGIC_STOP)
        {
            break;
        }

        printk("0x%p%p 0x%p%p %p ", raw.base_addr_high, raw.base_addr_low, raw.length_high, raw.length_low, raw.type);

        if (raw.type == ADDR_RANGE_MEMORY)
        {
            printk("[ Available ]\n");
            continue;
        }

        printk("[ Reserved ]\n");
    }
}


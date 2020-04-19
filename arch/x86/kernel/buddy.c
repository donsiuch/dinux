
#include "dinux/inc/memory.h"
#include "dinux/inc/list.h"
#include "dinux/inc/io.h"
#include "x86/inc/arch_mm.h"
#include "x86/inc/buddy.h"

extern void kernel_bug(void);

extern struct page *physical_page_ledger;
extern struct memory_stats mem_stats;

static struct mem_node node;

static inline int get_block_size(int order)
{
    return (power(2, order));
}

static inline int calc_buddy_idx(int idx, int order)
{
    return (idx ^ (1 << order));
}

//
// TODO: I think this is wrong
//
static inline int is_left_buddy(int idx)
{
    return ((~idx) & 1);
}

static int is_buddy_pages_free(int idx, int order)
{
    int i;

    for (i = 0; i < get_block_size(order); i++)
    {
        if (physical_page_ledger[idx + i].count > 0)
        {
            return 0;
        }
    }

    return 1;
}

static void set_order_bitmap(int idx, int order)
{
    int i;
    int limit = idx + 2*get_block_size(order);

    if (limit >= mem_stats.nr_total_frames)
       limit = mem_stats.nr_total_frames-1;

    for (i = idx; i < limit; i++)
    {
        physical_page_ledger[i].order_bitmap = 1;
        physical_page_ledger[i].order_bitmap <<= order;
    }

//printk("%p \n", physical_page_ledger[mem_stats.nr_total_frames-1].order_bitmap);
}

#if 0
void split_block(struct page *page_ptr)
{

}

void alloc_pages(int num_pages)
{

}

void free_pages(int num_pages)
{

}
#endif

static void init_mem_node(struct mem_node *ptr)
{
    int i;

    memset(ptr, 0, sizeof(struct mem_node));

    for (i = 0; i < ZONE_MAX_NR ; i++)
    {
        memset(&(ptr->mem_zone[i]), 0, sizeof(struct mem_zone));
    }
}

static void add_to_free_list(struct mem_zone *zone_ptr, int order, struct list_head *lh_ptr)
{
    if (zone_ptr->free_list[order] == NULL)
    {
        init_list_head(lh_ptr);

        zone_ptr->free_list[order] = lh_ptr;

        return;
    }
    
    append_list(zone_ptr->free_list[order], lh_ptr);
}

void set_all_pages_to_zero_order()
{
    int i;

    for (i = 0; i < mem_stats.nr_total_frames; i++)
    {
        physical_page_ledger[i].order_bitmap = 1;
    }
}

void merge_blocks(int i, int buddy_i)
{
    physical_page_ledger[i].order_bitmap <<= 1;
    physical_page_ledger[buddy_i].order_bitmap <<= 1;
    
    //printk("%s: block = %p, buddy_i = %p, block bitmap = %p, count = %p,  buddy bitmap = %p, buddy count = %p\n", __func__, i << 12, buddy_i << 12, physical_page_ledger[i].order_bitmap, physical_page_ledger[i].count, physical_page_ledger[buddy_i].order_bitmap, physical_page_ledger[buddy_i].count);
}

static int get_order(unsigned long bitmap)
{
    unsigned long map = bitmap;
    unsigned long order = 0;

    while ( map > 1 )
    {
        map >>= 1;
        order++;
    }

    return order;
}

//
// TODO: This is recursive. Make it iterative for speed.
//
static int _free_buddy(int i)
{
    int ret = 0;
    int index = 0;
    int buddy_i = 0;

    if (physical_page_ledger[i].count != 0)
    {
        ret = get_order(physical_page_ledger[i].order_bitmap);
        goto exit;
    }

    buddy_i = calc_buddy_idx(i, get_order(physical_page_ledger[i].order_bitmap));

//printk("A. i = %p, buddy_i = %p, order = %p\n", i, buddy_i, get_order(physical_page_ledger[i].order_bitmap));
    
    while ( physical_page_ledger[buddy_i].count == 0 &&
            (get_order(physical_page_ledger[i].order_bitmap) < BUDDY_MAX_ORDER) )
    {

printk("B. i = %p, buddy_i = %p, order = %p\n", i, buddy_i, get_order(physical_page_ledger[i].order_bitmap));

        if ((physical_page_ledger[i].order_bitmap == 
                 physical_page_ledger[buddy_i].order_bitmap))
        {
            if ((buddy_i = calc_buddy_idx(i, get_order(physical_page_ledger[i].order_bitmap))) < i)
            {
                index = buddy_i;
            }
            else
            {
                merge_blocks(i, buddy_i);
                index = i;
            }
        }
        else if (ret > 0)
        {
            printk("Go to free block\n");
            goto free_block;
        }
        else
        {
            index = buddy_i;
        }
 
        ret = _free_buddy(index);
        if (ret > 0 && (index != buddy_i))
        {
            goto exit; 
        }

        buddy_i = calc_buddy_idx(i, get_order(physical_page_ledger[i].order_bitmap));
    }

    if (ret > 0)
    {
        printk("A lower layer then me freed\n");
        goto exit; 
    }

free_block:
    add_to_free_list(&node.mem_zone[ZONE_NORMAL], get_order(physical_page_ledger[i].order_bitmap), &physical_page_ledger[i].list);
    printk("%s: @ Added block = %p to free list #%p\n", __func__, i << 12, get_order(physical_page_ledger[i].order_bitmap));
    ret = physical_page_ledger[i].order_bitmap;

exit:
    return ret;
}

//
// We assume that we will always have the 
//
static int free_buddy(int i)
{
        int buddy_i;

        buddy_i = calc_buddy_idx(i, get_order(physical_page_ledger[i].order_bitmap));

        if (buddy_i < i)
        {
           return  _free_buddy(buddy_i);
        }

        return _free_buddy(i);
}

/*
 * Name:    setup_buddy()
 *
 * Loop through the page ledger and add free frames to the free buddy lists
 *
 */
void setup_buddy()
{
    int i = 0;
    
    // delete this is for a test
    int x = 0;

    init_mem_node(&node);

    set_all_pages_to_zero_order();

#if 0
    // delete this while loop
    while (i < mem_stats.nr_total_frames && x < 1)
    {
        if (physical_page_ledger[i].count > 0)
        {
            i++;
            continue;
        }

        //printk("i = %p, order = %p, buddy i = %p\n", i+2, physical_page_ledger[i].order_bitmap, calc_buddy_idx(i+2, 2));

        //physical_page_ledger[i].count += 1;

        free_buddy(i+4);
       
        

        free_buddy(i+4);

        // delete this is a test
        x++;
    }
#endif


    while (i < mem_stats.nr_total_frames && x < 1)
    //while (i < mem_stats.nr_total_frames)
    {
        if (physical_page_ledger[i].count > 0)
        {
            i++;
            continue;
        }

        physical_page_ledger[i+5].count = 1;

        //
        // All pages are the 0th order
        //
        i += power(2, get_order(_free_buddy(i)));

        // delete this is a test
        x++;
    }

    while(1){}
}

static struct list_head * del_from_free_list(struct mem_zone *zone_ptr, int order)
{
    struct list_head *lh_ptr;

    //
    // Record the item to delete
    //
    lh_ptr = zone_ptr->free_list[order];

    if (zone_ptr->free_list[order]->next == lh_ptr)
    {
        zone_ptr->free_list[order] = NULL;
    }
    else
    {
        zone_ptr->free_list[order] = lh_ptr->next;
    }

    del_list(lh_ptr);

    printk("Removed from list = %p, new list head = %p\n", lh_ptr, zone_ptr->free_list[order]);

    return lh_ptr;
}

void split_block(int order)
{
    struct list_head *lh_ptr = del_from_free_list(&node.mem_zone[ZONE_NORMAL], order);
    struct page *ptr = container_of(lh_ptr, struct page, list); 
    
    printk(">> order = %p, ptr->order_bitmap = %p\n", order, ptr->order_bitmap);

    struct page *ptr2 = (struct page *)calc_buddy_idx(ptr, order - 1 );
    printk("ptr = %p, ptr2 = %p bitmap of second block = %p\n", ptr, ptr2, ptr2->order_bitmap);

}

void split_blocks_to_order(int order)
{
    int i;
    for (i = order; i < BUDDY_MAX_ORDER; i++)
    {
        if (node.mem_zone[ZONE_NORMAL].free_list[i] == NULL)
        {
            printk("No blocks at this order.\n");
            continue;
        }

        if (i == order)
        {
            printk("Found a block, returning it.\n");
            return del_from_free_list(&node.mem_zone[ZONE_NORMAL], i);
        }
        
        printk("Found a larger block that needs to be split at order = %p\n", i);
        return split_block(i); 
    }

    printk("No blocks in memory!\n");
    kernel_bug();
}

int allocate_buddy(int num_pages)
{
    int order;

    //
    // Find the appropriate order that can handle the request
    //
    for(order = 0; order < BUDDY_MAX_ORDER; order++)
    {
        printk("order = %p, num_pages = %p, block_size = %p\n", order, num_pages, get_block_size(order));
        if (num_pages <= get_block_size(order))
            break;
    }

    if (node.mem_zone[ZONE_NORMAL].free_list[order] != NULL)
    {
       split_blocks_to_order(order); 
    }
    else
    {
        printk("%s: No blocks in memory!\n",  __func__);
        kernel_bug();    
    }

    return 0;
}


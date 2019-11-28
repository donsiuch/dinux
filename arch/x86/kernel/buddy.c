
#include "dinux/inc/memory.h"
#include "dinux/inc/list.h"
#include "dinux/inc/io.h"
#include "x86/inc/arch_mm.h"
#include "x86/inc/buddy.h"

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
void add_block_to_list(struct page *page_ptr, int order)
{

}

void split_block(struct page *page_ptr)
{

}

void merge_block(struct page *page_ptr)
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
        memset(&ptr->mem_zone[i], 0, sizeof(struct mem_zone));
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

    add_list(zone_ptr->free_list[order], lh_ptr);
}

void setup_buddy()
{

    unsigned long i = 0;
    int buddy_i;
    int order;

    init_mem_node(&node);

    while (i < mem_stats.nr_total_frames)
    {
        if (physical_page_ledger[i].count > 0)
        {
            i++;
            continue;
        }

        //
        // We found a page that has the potential to coalesce
        //

        //
        // Coalesce as high as we can go
        //
        for (order = 0; order < BUDDY_MAX_ORDER; order++)
        {
            buddy_i = calc_buddy_idx(i, order);

            if ((buddy_i + get_block_size(order)) > mem_stats.nr_total_frames)
            {
                printk("End of memory i = %p, buddy = %p, block size = %p, order = %p,left? = %p\n", i, buddy_i, power(2,order), order, is_left_buddy(i));
                
                // TODO: get rid of the while loop
                //kernel_bug();

                while(1){}
            }
       
            //
            // Check all the buddy's pages
            //
            if (is_buddy_pages_free(buddy_i, order) == 0)
            {
                break;
            }
        }

        order -= 1;

        add_to_free_list(&node.mem_zone[ZONE_NORMAL], order, &physical_page_ledger[i].list);

        set_order_bitmap(i, order);

        //
        // Skip past all the free blocks from both buddies
        //
        i += 2*get_block_size(order);
    }
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
        if (node.mem_zone[ZONE_NORMAL].free_list[i] != NULL)
            break;
    }

    split_block(i);
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
printk("%p\n", order);
    if (node.mem_zone[ZONE_NORMAL].free_list[order] == NULL)
    {
       split_blocks_to_order(order); 
    }
printk("xxx\n");
    return 0;
}


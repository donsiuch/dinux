
#ifndef __BUDDY__
#define __BUDDY__

#include <stdint.h>

#define MEM_ZONE_DMA_MAX_ADDR 0x1000000
typedef enum {

    // <= 0x01000000
    ZONE_DMA = 0,

    // > 0x01000000
    ZONE_NORMAL = 1,
    ZONE_MAX_NR,
} ZONE_T;

#define BUDDY_MAX_ORDER 8
struct mem_zone {
    //
    // [0] = 2^0 = 1x page
    // [1] = 2^1 = 2x pages
    // [2] = 2^2 = 4x pages
    // [3] = 2^3 = 8x pages
    // etc.
    //
    struct list_head *free_list[BUDDY_MAX_ORDER]; 
};

struct mem_node {
    struct mem_zone mem_zone[ZONE_MAX_NR];
};

void setup_buddy();

#endif


#ifndef __MEME820__
#define __MEME820__

#define MEME820_ADDR 0x9000

// Available for OS use
#define ADDR_RANGE_MEMORY 1

// Reserved for the system (non OS use)
#define ADDR_RANGE_RESERVED 2

#ifndef ASSEMBLY

struct meme820 {
    unsigned long base_addr_low;
    unsigned long base_addr_high;
    unsigned long length_low;
    unsigned long length_high;
    unsigned long type;
} __attribute((packed));

#endif

#endif

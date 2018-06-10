
#ifndef __MEME820__
#define __MEME820__

#define MEME820_ADDR 0x9000
#define MEME820_MAX_NR 1024

// Available for OS use
#define ADDR_RANGE_MEMORY 1

// Reserved for the system (non OS use)
#define ADDR_RANGE_RESERVED 2

#define MEME820_MAGIC_STOP 0x11041985

#ifndef ASSEMBLY

struct meme820 {
    unsigned long base_addr_low;
    unsigned long base_addr_high;
    unsigned long length_low;
    unsigned long length_high;
    unsigned long type;
} __attribute((packed));

// Size of single result
#define MEME820_RESULT_SIZE sizeof(struct meme820)

void sanitize_meme820_map(void);

#endif

#endif

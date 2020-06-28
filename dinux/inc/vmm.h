
#ifndef __VMM_H__
#define __VMM_H__

#include <stdint.h>

#include "dinux/inc/math.h"

//
// Page aligned
//
struct vm_area {
	unsigned long start;
	unsigned long size;
	struct vm_area *next_ptr;
};

#define VMM_MAX_HEAP_IDX 4
struct heap_index {
	
	//
	// [0] = 2^4 = 16
	// [1] = 2^5 = 32
	// [2] = 2^6 = 64
    // [3] = 2^7 = 128
	//
	struct heap_head *heap_head[4];
};

struct heap_head {
	uint32_t size;
	struct chunk_head *free_chunk_ptr;	
};

#define VMM_CHUNK_IN_USE 0xBAAAAAAA

struct chunk_head {
	uint32_t size;

    //
    // Special value that states whether or not a chunk of memory is free
    // If this value is equal to the configured magic number, the value is free
    // and previous and next pointers can be used
    //
	uint32_t magic;

} __attribute__((packed));

//
// Only used if the magic number in chunk head is correct
//
struct free_chunk_data {
	struct chunk_head *prev_ptr;
	struct chunk_head *next_ptr;
};

#define VMM_CHUNK_DATA(_x)((struct free_chunk_data *)((unsigned long)_x + sizeof(struct chunk_head)))

void *kmalloc(unsigned long, unsigned long);
void setup_heap(void);

#endif

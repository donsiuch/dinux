
#ifndef __VMM_H__
#define __VMM_H__

#include <stdint.h>

void *kmalloc(unsigned long, unsigned long);

struct vm_area {
	unsigned long virt_start;
	unsigned long virt_end;
	struct vm_area *next_ptr;
};

struct heap_index {
	
	//
	// [0] = 2^4 = 16
	// [1] = 2^5 = 32
	// [2] = 2^6 = 64
	//
	struct heap *array_of_heaps[4];
};

struct heap {
	uint32_t size;
	struct chunk_head *free_chunk_ptr;	
};

struct chunk_head {
	uint32_t size;
	uint32_t magic;
};

struct free_chunk_data {
	struct chunk_head *prev_ptr;
	struct chunk_head *next_ptr;
};

void setup_heap(unsigned long);

#endif

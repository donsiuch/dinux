
#ifndef __VMM_H__
#define __VMM_H__

void *kmalloc(unsigned long, unsigned long);

struct vm_area {
	unsigned long virt_start;
	unsigned long virt_end;
};

#endif

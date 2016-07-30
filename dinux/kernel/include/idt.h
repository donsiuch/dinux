
#ifndef __IDT_HEADER__
#define __IDT_HEADER__

#define MAX_IDT_ENTRIES 256

typedef struct {
   unsigned short offset_1; // offset bits 0..15
   unsigned short selector; // a code segment selector in GDT or LDT
   unsigned char zero;      // unused, set to 0
   unsigned type_attr; // type and attributes, see below
   unsigned short offset_2; // offset bits 16..31
} __attribute__((packed)) idtDescriptor; 

idtDescriptor idt[MAX_IDT_ENTRIES];

int idtSize = MAX_IDT_ENTRIES*sizeof(idtDescriptor);
int seven = 7;

#endif


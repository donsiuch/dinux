
#ifndef __IDT_HEADER__
#define __IDT_HEADER__

#define MAX_IDT_ENTRIES 256

typedef struct {
   unsigned short routineAddressLower; // offset bits 0..15
   unsigned short selector; // a code segment selector in GDT or LDT
   unsigned char zero;      // unused, set to 0
   unsigned char type_attr; // type and attributes, see below
   unsigned short routineAddressUpper; // offset bits 16..31
} __attribute__((packed)) idtDescriptor; 

typedef struct
{
    unsigned int gs, fs, es, ds;      
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; 
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
} regs;

idtDescriptor idt[MAX_IDT_ENTRIES];

int idtSize = MAX_IDT_ENTRIES*sizeof(idtDescriptor);

#endif


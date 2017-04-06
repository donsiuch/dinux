
#ifndef __IDT_HEADER__
#define __IDT_HEADER__

#include "./kernel_defs.h"

#define MAX_IDT_ENTRIES 256

/*

   
 Type attribute field:
     7                           0
   +---+---+---+---+---+---+---+---+
   | P |  DPL  | S |    GateType   |
   +---+---+---+---+---+---+---+---+	

 Bit 7:
 	P = Present - set to 0 for unused interrupts.
 Bits 6-5:
 	DPL = Descriptor privlege level ( ring 0 -3 )
 Bit 4:	
	S = storage segment - set to 0 for intterupt gates
 Bits 3-0
 	Gate Type:
	0101 	0x5 	5 	80386 32 bit task gate
	0110 	0x6 	6 	80286 16-bit interrupt gate
	0111 	0x7 	7 	80286 16-bit trap gate
	1110 	0xE 	14 	80386 32-bit interrupt gate
	1111 	0xF 	15 	80386 32-bit trap gate
 */
typedef struct {
   unsigned short routineAddressLower; // offset bits 0..15
   unsigned short selector; // a code segment selector in GDT or LDT
   unsigned char zero;      // unused, set to 0
   unsigned char type_attr; // type and attributes, see below
   unsigned short routineAddressUpper; // offset bits 16..31
} __attribute__((packed)) idtDescriptor; 

typedef struct
{
    // Manually pushed
    unsigned int gs, fs, es, ds;      
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; 

    // Implicitly pushed.
    unsigned int err_code; // Might not be pushed
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int useresp;
    unsigned int ss;
} regs;

idtDescriptor idt[MAX_IDT_ENTRIES];

int idtSize = MAX_IDT_ENTRIES*sizeof(idtDescriptor);

extern void placeHolder(void);
extern void divideErrorIsr(void);
asmlinkage void doDivideError(unsigned char *, regs);

#endif


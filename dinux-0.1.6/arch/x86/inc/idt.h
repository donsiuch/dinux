
#ifndef __IDT_H__
#define __IDT_H__

#include "./kernel_defs.h"

#define MAX_IDT_ENTRIES 256

#define TRAP_GATE		0x8F
#define	INTERRUPT_GATE		0x8E
#define SYSTEM_INTERRUPT_GATE	0xFE
#define	SYSTEM_GATE		0xEF

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
   unsigned char type_attr; // type and attributes
   unsigned short routineAddressUpper; // offset bits 16..31
} __attribute__((packed)) idtDescriptor; 

typedef struct
{
    unsigned int gs, fs, es, ds;      
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; 
    unsigned int errorCode;
    unsigned int eip, cs, eflags, useresp, ss; 
} __attribute__((packed))regs;

idtDescriptor idt[MAX_IDT_ENTRIES];

int idtSize = MAX_IDT_ENTRIES*sizeof(idtDescriptor);

extern void unknownFault(void);
extern void divideError(void);
extern void debug(void);
extern void nmi(void);
extern void breakPoint(void);
extern void overflow(void);
extern void boundaryVerification(void);
extern void invalidOpcode(void);
extern void deviceNotAvail(void);
extern void doubleFault(void);
extern void coProcSegOverrun(void);
extern void invalTss(void);
extern void segNotPresent(void);
extern void stackException(void);
extern void generalProtection(void);
extern void pageFault(void);
extern void floatError(void);
extern void alignmentCheck(void);
extern void machineCheck(void);
extern void simdFloatException(void);
extern void virtException(void);
extern void systemTimer(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);
extern void systemCall(void);

asmlinkage void doUnknownFault(regs *);
asmlinkage void doDivideError(regs *);
asmlinkage void doDebug(regs *);
asmlinkage void doNmi(regs *);
asmlinkage void doBreakPoint(regs *);
asmlinkage void doOverflow(regs *);
asmlinkage void doBoundaryVerification(regs *);
asmlinkage void doInvalidOpcode(regs *);
asmlinkage void doDeviceNotAvail(regs *);
asmlinkage void doDoubleFault(regs *);
asmlinkage void doCoProcSegOverrun(regs *);
asmlinkage void doInvalTss(regs *);
asmlinkage void doSegNotPresent(regs *);
asmlinkage void doStackException(regs *);
asmlinkage void doGeneralProtection(regs *);
asmlinkage void doPageFault(regs *);
asmlinkage void doFloatError(regs *);
asmlinkage void doAlignmentCheck(regs *);
asmlinkage void doMachineCheck(regs *);
asmlinkage void doSIMDFloatException(regs *);
asmlinkage void doVirtException(regs *);
asmlinkage void doSystemTimer(regs *);
asmlinkage void doIrq1(regs *);
asmlinkage void doIrq2(regs *);
asmlinkage void doIrq3(regs *);
asmlinkage void doIrq4(regs *);
asmlinkage void doIrq5(regs *);
asmlinkage void doIrq5(regs *);
asmlinkage void doIrq6(regs *);
asmlinkage void doIrq7(regs *);
asmlinkage void doIrq8(regs *);
asmlinkage void doIrq9(regs *);
asmlinkage void doIrq10(regs *);
asmlinkage void doIrq11(regs *);
asmlinkage void doIrq12(regs *);
asmlinkage void doIrq13(regs *);
asmlinkage void doIrq14(regs *);
asmlinkage void doIrq15(regs *);
asmlinkage void doSystemCall(regs *);

static void dumpRegisters(regs *);

#endif


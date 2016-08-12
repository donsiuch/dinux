
#include "../include/idt.h"
#include "../../utility/include/io.h"
#include "../../utility/include/memoryOperations.h"

void handleFault( unsigned char *stackPtr, regs registers )
{
	printd("handleFault\n");
}

void setGate(unsigned short index, unsigned long routineAddress, unsigned short selector, unsigned char type_attr)
{
	idt[index].routineAddressLower = routineAddress & 0xFFFF;
	idt[index].routineAddressUpper = (routineAddress >> 16) & 0xFFFF;

	// GDT selector. Should be KERNEL_CS?
	idt[index].selector = selector;

	idt[index].type_attr = type_attr;
}

extern void isrCommon(void);

/*
 * Function: populateIdt
 *
 * 
 *
 */
void populateIdt()
{
	unsigned short index = 0;

	memset(idt, 0, sizeof(idt));

	while ( index < MAX_IDT_ENTRIES ) 
	{
		// 0x8E == 1000 1110
		// *Is routine present? (yes) = 1
		// Descriptor Privledge level bit 1 = 0
		// Descriptor Privledge level bit 2 = 0
		// ? Storage segment ( 0 for interrupt) = 0
		// 
		// * Causes crash if 0
		//
		//
		setGate(index,(unsigned long)isrCommon, 0x10, 0x8E);
		index ++;
	}

/*
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	setGate(0, (unsigned long *)isr0, 0x08, 0x0E);
	*/
}


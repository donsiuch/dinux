
#include "../include/idt.h"
#include "../../utility/include/io.h"
#include "../../utility/include/memoryOperations.h"

/*
 * Function: handleFault
 *
 *
 */
void handleFault( unsigned char *stackPtr, regs registers )
{
	printd("handleFault\n");
}

/*
 * Function: setGate
 *
 *
 */
void setGate(unsigned short index, unsigned long routineAddress, unsigned short selector, unsigned char type_attr)
{
	idt[index].routineAddressLower = routineAddress & 0xFFFF;
	idt[index].routineAddressUpper = (routineAddress >> 16) & 0xFFFF;
	idt[index].selector = selector;
	idt[index].type_attr = type_attr;
}

/*
 * Function: populateIdt
 *
 * 
 *
 */
void populateIdt()
{
	unsigned short index = 1;

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
		setGate(index,(unsigned long)placeHolder, KERNEL_CS, 0x8F);
		index ++;
	}
	
	setGate(0, (unsigned long *)divideErrorIsr, KERNEL_CS, 0x8F);
	
}

// Traps v Interrupts: http://stackoverflow.com/questions/3425085/the-difference-between-call-gate-interrupt-gate-trap-gate 
//
// Source: http://phrack.org/issues/59/4.html
// --------------------------------------------------------------------------+
// number  | Exception 			   | Exception Handler               |
// --------------------------------------------------------------------------+
// 0       | Divide Error		   | divide_error() - trap gate      
// 1       | Debug			   | debug() - trap gate             
// 2       | Nonmaskable Interrupt  	   | nmi() - trap gate              
// 3       | Break Point		   | int3() - ring 3
// 4       | Overflow			   | overflow() - ring 3
// 5       | Boundary verification	   | bounds() - ring 3 
// 6       | Invalid operation code	   | invalid_op() - trap gate
// 7       | Device not available	   | device_not_available() - trap gate
// 8       | Double Fault                  | double_fault() - trap gate
// 9       | Coprocessor segment overrun   | coprocesseur_segment_overrun() - trap gate
// 10      | TSS not valid	    	   | invalid_tss() - trap gate
// 11      | Segment not  present	   | segment_no_present() - trap gate
// 12      | stack exception 		   | stack_segment() - trap gate
// 13      | General Protection 	   | general_protection() - trap gate
// 14 	   | Page Fault			   | page_fault() - trap gate
// 15      | Reserved by Intel		   | none
// 16      | Calcul Error with float virgul| coprocessor_error() - trap gate
// 17      | Alignement check		   | alignement_check() - trap gate
// 18      | Machine Check		   | machine_check() - trap gate
//
// ...
// 
// 32	   | IRQ0 ( maskable )
// ...
// 47      | IRQ15 ( maskable )
// 
// 0x80    | System Call		   | system_call() - ring 3
// --------------------------------------------------------------------------+
//

/*
 * Function: doDivideError
 *
 * 
 *
 */
asmlinkage void doDivideError(unsigned char *stackPtr, regs registers)
{
	printd("do_divide_error()\n");
}

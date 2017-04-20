
#include "../include/idt.h"
#include "../../utility/include/io.h"
#include "../../utility/include/memoryOperations.h"

/*
 * Function: handleFault
 *
 *
 */
void handleFault(regs registers, unsigned int errorCode)
{
	printd("handleFault, %p %p\n", registers.eax, errorCode);
	__asm__("hlt");
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
	unsigned short index = 0;

	memset(idt, 0, sizeof(idt));

	// to-do: re-purpose loop for the IRQs and the rest of the IDT
	while ( index < MAX_IDT_ENTRIES ) 
	{
		setGate(index,(unsigned long)placeHolder, KERNEL_CS, TRAP_GATE);
		index ++;
	}

	setGate(0, (unsigned long)divideError, KERNEL_CS, TRAP_GATE);
	setGate(1, (unsigned long)debug, KERNEL_CS, TRAP_GATE);
	setGate(2, (unsigned long)nmi, KERNEL_CS, INTERRUPT_GATE); 
	setGate(3, (unsigned long)breakPoint, KERNEL_CS, TRAP_GATE);
	setGate(4, (unsigned long)overflow, KERNEL_CS, TRAP_GATE);
	setGate(5, (unsigned long)boundaryVerification, KERNEL_CS, TRAP_GATE);
	setGate(6, (unsigned long)invalidOpcode, KERNEL_CS, TRAP_GATE); 
	setGate(7, (unsigned long)deviceNotAvail, KERNEL_CS, TRAP_GATE);
	setGate(8, (unsigned long)doubleFault, KERNEL_CS, TRAP_GATE);
	setGate(9, (unsigned long)coProcSegOverrun, KERNEL_CS, TRAP_GATE);
	setGate(10, (unsigned long)invalTss, KERNEL_CS, TRAP_GATE); 
	setGate(11, (unsigned long)segNotPresent, KERNEL_CS, TRAP_GATE);
	setGate(12, (unsigned long)stackException, KERNEL_CS, TRAP_GATE);
	setGate(13, (unsigned long)generalProtection, KERNEL_CS, TRAP_GATE);
	setGate(14, (unsigned long)pageFault, KERNEL_CS, TRAP_GATE); 
	// 15 unused. Reserved by Intel
	setGate(15, (unsigned long)alignmentCheck, KERNEL_CS, TRAP_GATE);
	setGate(16, (unsigned long)machineCheck, KERNEL_CS, TRAP_GATE);
	setGate(17, (unsigned long)simdFloatException, KERNEL_CS, TRAP_GATE); 
	setGate(18, (unsigned long)virtException, KERNEL_CS, TRAP_GATE);
	// ... ?
	setGate(32, (unsigned long)systemTimer, KERNEL_CS, INTERRUPT_GATE);

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
 * Exception class: Fault
 * Vector: #0
 *
 * Stack: A dummy error code was pushed for this case in the assembly caller. It is not needed though.
 *
 */
asmlinkage void doDivideError(regs registers)
{
	printd("doDivideError(): %p, &registers: %p\n", doDivideError, &registers);

	// to-do:
	//	1. Kill current
	//	2. Re-schedule
	//	3. We must hlt if we came from ring 0...
	//		call kernel die() routine.

	__asm__("hlt");
		
}

asmlinkage void doDebug(regs registers)
{
	printd("doDebug(): %p, &registers: %p\n", doDebug, &registers);

	__asm__("hlt");
}

asmlinkage void doNmi(regs registers)
{
	printd("doNmi(): %p, &registers: %p\n", doNmi, &registers);
	__asm__("hlt");
}

asmlinkage void doBreakPoint(regs registers)
{
	printd("doBreakPoint(): %p, &registers: %p\n", doBreakPoint, &registers);
	__asm__("hlt");
}

asmlinkage void doOverflow(regs registers)
{
	printd("doOverflow(): %p, &registers: %p\n", doOverflow, &registers);
	__asm__("hlt");
}

asmlinkage void doBoundaryVerification(regs registers)
{
	printd("doBoundaryVerification(): %p, &registers: %p\n", doBoundaryVerification, &registers);
	__asm__("hlt");
}

asmlinkage void doInvalidOpcode(regs registers)
{
	printd("doInvalidOpcode(): %p, &registers: %p\n", doInvalidOpcode, &registers);
	__asm__("hlt");
}

asmlinkage void doDeviceNotAvail(regs registers)
{
	printd("doDeviceNotAvail(): %p, &registers: %p\n", doDeviceNotAvail, &registers);
	__asm__("hlt");
}

asmlinkage void doDoubleFault(regs registers, unsigned int errorCode)
{
	int x = 0x77777777;
	printd("doDoubleFault(): %p, errorCode: %p, &registers: %p\n", doDoubleFault, errorCode, &registers);
	dumpBytes(&x, 128);

	// Automatically aborts (hlts)
}

asmlinkage void doCoProcSegOverrun(regs registers)
{
	printd("doCoProcSegOverrun(): %p, &registers: %p\n", doCoProcSegOverrun, &registers);
	__asm__("hlt");
}

asmlinkage void doInvalTss(regs registers, unsigned int errorCode)
{
	printd("doInvalTss(): %p, errorCode: %p, &registers: %p\n", doInvalTss, errorCode, &registers);
	__asm__("hlt");
}

asmlinkage void doSegNotPresent(regs registers, unsigned int errorCode)
{
	printd("doSegNotPresent(): %p, errorCode: %p, &registers: %p\n", doSegNotPresent, errorCode, &registers);
	__asm__("hlt");
}

// Seg fault
asmlinkage void doStackException(regs registers, unsigned int errorCode)
{
	printd("doStackException(): %p, errorCode: %p, &registers: %p\n", doStackException, errorCode, &registers);
	__asm__("hlt");
}

asmlinkage void doGeneralProtection(regs registers, unsigned int errorCode)
{
	printd("doGeneralProtection(): %p, errorCode: %p, &registers: %p\n", doGeneralProtection, errorCode, &registers);
	__asm__("hlt");
}

asmlinkage void doPageFault(regs registers, unsigned int errorCode)
{
	printd("doPageFault(): %p, errorCode: %p, &registers: %p\n", doPageFault, errorCode, &registers);
	__asm__("hlt");
}

asmlinkage void doFloatError(regs registers)
{
	printd("doFloatError(): %p, &registers: %p\n", doFloatError, &registers);
	__asm__("hlt");
}

asmlinkage void doAlignmentCheck(regs registers, unsigned int errorCode)
{
	printd("doAlignmentCheck(): %p, errorCode: %p, &registers: %p\n", doAlignmentCheck, errorCode, &registers);
	__asm__("hlt");
}

asmlinkage void doMachineCheck(regs registers)
{
	printd("doMachineCheck(): %p, &registers: %p\n", doMachineCheck, &registers);
	__asm__("hlt");
}

asmlinkage void doSIMDFloatException(regs registers)
{
	printd("doSIMDFloatException(): %p, &registers: %p\n", doSIMDFloatException, &registers);
	__asm__("hlt");
}

asmlinkage void doVirtException(regs registers)
{
	printd("doVirtException(): %p, &registers: %p\n", doVirtException, &registers);
	__asm__("hlt");
}

asmlinkage void doSystemTimer(regs registers)
{
	printd("doSystemTimer(): %p, &registers: %p\n", doSystemCall, &registers);
	__asm__("hlt");
}

asmlinkage void doSystemCall(regs registers)
{
	printd("doSystemCall(): %p, &registers: %p\n", doSystemCall, &registers);
	__asm__("hlt");
}



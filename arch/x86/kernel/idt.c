
#include "x86/inc/arch_mm.h"
#include "x86/inc/idt.h"
#include "dinux/inc/io.h"
#include "dinux/inc/memory.h"
#include "x86/inc/pic.h"
#include "x86/inc/pit.h"
#include "x86/inc/system.h"
#include "x86/inc/time.h"

/*
 * Function: setGate()
 * 
 * Parameters:
 *	index - Interrupt descriptor table index
 *	routineAddress - Address of the service routine
 *	selector - Code segment
 *	type_attr - See header file for details.
 *
 * Description: Create an entry in the idt
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
 * Function: populateIdt()
 *
 * Parameters:
 *	void
 *
 * Description: 
 *	Remaps the hardware IRQs so they do not overlap the software IRQs.
 *	Populates the interrupt descriptor table.
 *
 */
void populateIdt()
{
	unsigned short index = 0;

	memset(idt, 0, sizeof(idt));

	// Initialize the timer to 100Hz
	initializePit(100);

	// Map the IRQs to prevent overlapping with software interrupts
	remapIrq();

	// Fill the the IDT with the generic fault handler 
	while ( index < MAX_IDT_ENTRIES ) 
	{
		setGate(index,(unsigned long)unknownFault, KERNEL_CS, TRAP_GATE);
		index ++;
	}

	// Now fill the table with the fault handlers we care about
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
	// ... 
	setGate(32, (unsigned long)systemTimer, KERNEL_CS, INTERRUPT_GATE);
	setGate(33, (unsigned long)irq1, KERNEL_CS, INTERRUPT_GATE);
	setGate(34, (unsigned long)irq2, KERNEL_CS, INTERRUPT_GATE);
	setGate(35, (unsigned long)irq3, KERNEL_CS, INTERRUPT_GATE);
	setGate(36, (unsigned long)irq4, KERNEL_CS, INTERRUPT_GATE);
	setGate(37, (unsigned long)irq5, KERNEL_CS, INTERRUPT_GATE);
	setGate(38, (unsigned long)irq6, KERNEL_CS, INTERRUPT_GATE);
	setGate(39, (unsigned long)irq7, KERNEL_CS, INTERRUPT_GATE);
	setGate(40, (unsigned long)irq8, KERNEL_CS, INTERRUPT_GATE);
	setGate(41, (unsigned long)irq9, KERNEL_CS, INTERRUPT_GATE);
	setGate(42, (unsigned long)irq10, KERNEL_CS, INTERRUPT_GATE);
	setGate(43, (unsigned long)irq11, KERNEL_CS, INTERRUPT_GATE);
	setGate(44, (unsigned long)irq12, KERNEL_CS, INTERRUPT_GATE);
	setGate(45, (unsigned long)irq13, KERNEL_CS, INTERRUPT_GATE);
	setGate(46, (unsigned long)irq14, KERNEL_CS, INTERRUPT_GATE);
	setGate(47, (unsigned long)irq15, KERNEL_CS, INTERRUPT_GATE);
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
 * Function: doUknownFault()
 *
 * Parameters:
 *	registers -  
 *
 * Description:
 *	This routine catches all unknown exceptions and interrupts.
 *	The function will dump all registers and halt.
 *
 */
asmlinkage void doUnknownFault(regs *registers)
{
	printk("Unknown exception! Halting!\n");
	dumpRegisters(registers);
	__asm__("hlt");
}

/*
 * Function: doDivideError
 * Exception class: Fault
 * Vector: #0
 *
 * 
 *
 */
asmlinkage void doDivideError(regs *registers)
{
	printk("doDivideError(): %p, &registers: %p\n", doDivideError, registers);

	// to-do:
	//	1. Kill current
	//	2. Re-schedule
	//	3. We must hlt if we came from ring 0...
	//		call kernel die() routine.

	__asm__("hlt");
		
}

asmlinkage void doDebug(regs *registers)
{
	printk("doDebug(): %p, &registers: %p\n", doDebug, registers);
	__asm__("hlt");
}

asmlinkage void doNmi(regs *registers)
{
	printk("doNmi(): %p, &registers: %p\n", doNmi, registers);
	__asm__("hlt");
}

asmlinkage void doBreakPoint(regs *registers)
{
	printk("doBreakPoint(): %p, &registers: %p\n", doBreakPoint, registers);
	__asm__("hlt");
}

asmlinkage void doOverflow(regs *registers)
{
	printk("doOverflow(): %p, &registers: %p\n", doOverflow, registers);
	__asm__("hlt");
}

asmlinkage void doBoundaryVerification(regs *registers)
{
	printk("doBoundaryVerification(): %p, &registers: %p\n", doBoundaryVerification, registers);
	__asm__("hlt");
}

asmlinkage void doInvalidOpcode(regs *registers)
{
	printk("doInvalidOpcode(): %p, &registers: %p\n", doInvalidOpcode, registers);
	__asm__("hlt");
}

asmlinkage void doDeviceNotAvail(regs *registers)
{
	printk("doDeviceNotAvail(): %p, &registers: %p\n", doDeviceNotAvail, registers);
	__asm__("hlt");
}

asmlinkage void doDoubleFault(regs *registers)
{
	printk("doDoubleFault(): %p, &registers: %p\n", doDoubleFault, registers);
	
	// Automatically halts
}

asmlinkage void doCoProcSegOverrun(regs *registers)
{
	printk("doCoProcSegOverrun(): %p, &registers: %p\n", doCoProcSegOverrun, registers);
	__asm__("hlt");
}

asmlinkage void doInvalTss(regs *registers)
{
	printk("doInvalTss(): %p, errorCode: %p, &registers: %p\n", doInvalTss, registers->errorCode, registers);
	__asm__("hlt");
}

asmlinkage void doSegNotPresent(regs *registers)
{
	printk("doSegNotPresent(): %p, errorCode: %p, &registers: %p\n", doSegNotPresent, registers->errorCode, registers);
	__asm__("hlt");
}

// Seg fault
asmlinkage void doStackException(regs *registers)
{
	printk("doStackException(): %p, errorCode: %p, &registers: %p\n", doStackException, registers->errorCode, registers);
	__asm__("hlt");
}

asmlinkage void doGeneralProtection(regs *registers)
{
	printk("doGeneralProtection(): %p, &registers: %p\n", doGeneralProtection, registers);
}

asmlinkage void doPageFault(regs *registers)
{
	uint32_t faulting_address = 0;
    uint32_t new_virt_addr = 0;

	__asm__ volatile ("movl %%cr2, %0": "=r"(faulting_address) ::);

	printk("doPageFault(): faulting address = %p, errorCode: %p, &registers: %p\n", faulting_address, registers->errorCode, registers);

    //new_virt_addr = alloc_page();

    printk("%s: new_virt_addr = %p\n", __func__, new_virt_addr);

}

asmlinkage void doFloatError(regs *registers)
{
	printk("doFloatError(): %p, &registers: %p\n", doFloatError, registers);
	__asm__("hlt");
}

asmlinkage void doAlignmentCheck(regs *registers)
{
	printk("doAlignmentCheck(): %p, errorCode: %p, &registers: %p\n", doAlignmentCheck, registers->errorCode, registers);
	__asm__("hlt");
}

asmlinkage void doMachineCheck(regs *registers)
{
	printk("doMachineCheck(): %p, &registers: %p\n", doMachineCheck, registers);
	__asm__("hlt");
}

asmlinkage void doSIMDFloatException(regs *registers)
{
	printk("doSIMDFloatException(): %p, &registers: %p\n", doSIMDFloatException, registers);
	__asm__("hlt");
}

asmlinkage void doVirtException(regs *registers)
{
	printk("doVirtException(): %p, &registers: %p\n", doVirtException, registers);
	__asm__("hlt");
}

asmlinkage void doSystemTimer(regs *registers)
{
	//printk("isr: %p, irr: %p\n", pic_get_isr(), pic_get_irr());
	
	// delete -- making warnings go away.
	uint32_t eax = registers->eax;
	eax++;

	_G_TICK++;

	if ( _G_TICK % 100 == 0 )
	{
		// Temporary test function
		emitOneSecond();
	}
	
	// Tell the master PIC we are done servicing the interrupt.	
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq1(regs *registers)
{
	printk("isr: %p, irr: %p\n", pic_get_isr(), pic_get_irr());

	// Tell the master PIC we are done servicing the interrupt.
	uint32_t eax = registers->eax;
	eax++;
	
	// Tell the master PIC we are done servicing the interrupt.
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq2(regs *registers)
{
	printk("doIrq2(): %p, &registers: %p\n", doIrq2, registers);

	// Tell the master PIC we are done servicing the interrupt.	
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq3(regs *registers)
{
	printk("doIrq3(): %p, &registers: %p\n", doIrq3, registers);

	// Tell the master PIC we are done servicing the interrupt.
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq4(regs *registers)
{
	printk("doIrq4(): %p, &registers: %p\n", doIrq4, registers);

	// Tell the master PIC we are done servicing the interrupt.
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq5(regs *registers)
{
	printk("doIrq5(): %p, &registers: %p\n", doIrq5, registers);

	// Tell the master PIC we are done servicing the interrupt.	
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq6(regs *registers)
{
	printk("doIrq6(): %p, &registers: %p\n", doIrq6, registers);
	
	// Tell the master PIC we are done servicing the interrupt.
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq7(regs *registers)
{
	printk("doIrq7(): %p, &registers: %p\n", doIrq7, registers);

	if ( !(pic_get_isr() & 0x07) )
	{
		printk("Spurious IRQ detected in master PIC!\n");
		return;
	}

	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq8(regs *registers)
{
	printk("doIrq8(): %p, &registers: %p\n", doIrq8, registers);

	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq9(regs *registers)
{
	printk("doIrq9(): %p, &registers: %p\n", doIrq9, registers);
	
	// Tell master and slave PICs we are finished servicing the interrupt.		
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq10(regs *registers)
{
	printk("doIrq10(): %p, &registers: %p\n", doIrq10, registers);
	
	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq11(regs *registers)
{
	printk("doIrq11(): %p, &registers: %p\n", doIrq11, registers);
	
	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq12(regs *registers)
{
	printk("doIrq12(): %p, &registers: %p\n", doIrq12, registers);

	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq13(regs *registers)
{
	printk("doIrq13(): %p, &registers: %p\n", doIrq13, registers);

	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq14(regs *registers)
{
	printk("doIrq14(): %p, &registers: %p\n", doIrq14, registers);

	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq15(regs *registers)
{
	printk("doIrq15(): %p, &registers: %p\n", doIrq15, registers);

	// If ISR is not 15 then we got a spurious IRQ.
	// Send the EOI to the master
	if ( !(pic_get_isr() & 0x0f) )
	{
		printk("Spurious IRQ detected in master PIC!\n");
		outb(PIC_MASTER_COMMAND, 0x20);
		return;
	}

	// Handle interrupt
	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}
	

asmlinkage void doSystemCall(regs *registers)
{
	printk("doSystemCall(): %p, &registers: %p\n", doSystemCall, registers);
	__asm__("hlt");
}

static void dumpRegisters(regs *registers)
{
        printk("gs: %x, ", registers->gs);
        printk("fs: %x, ", registers->fs);
        printk("es: %x, ", registers->es);
        printk("ds: %x\n", registers->ds);
        printk("edi: %p, ", registers->edi);
        printk("esi: %p, ", registers->esi);
        printk("ebp: %p, ", registers->ebp);
        printk("esp: %p\n", registers->esp);
        printk("ebx: %p, ", registers->ebx);
        printk("edx: %p, ", registers->edx);
        printk("ecx: %p, ", registers->ecx);
        printk("eax: %p\n", registers->eax);
	printk("errorCode: %p\n", registers->errorCode);
	printk("eip: %p\n", registers->eip);
	printk("cs: %p, ", registers->cs);
	printk("eflags: %p, ",registers->eflags);
	printk("useresp: %p, ", registers->useresp);
	printk("ss: %p\n", registers->ss);
}

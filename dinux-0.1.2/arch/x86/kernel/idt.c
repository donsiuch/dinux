
#include "x86/inc/idt.h"
#include "x86/inc/system.h"
#include "dinux/inc/io.h"
#include "dinux/inc/memoryOperations.h"

/*
 * The following function reprograms the 8259A daisychained master and slave.
 * I also wrote a bunch of information on it because it is confusing as hell,
 * non-intuitive and there is very little documenation.
 *
 * The reason the PIC must be re-programmed is because the BIOS sets the PIC up in real mode
 * and when switching to protected mode, hardware intterupts that come in on the PIC
 * will hit CPU software vectors. Thus, hardware intterrupts must be remapped until after
 * the software vectors.
 *
 * 8259A PIC
 * The following information is derived from the Bochs 8259A data sheet + OSdevwiki
 * 
 * Interrupt (INT) - Signal that a device is ready to deliver data to the CPU. 
 * Interrupt Request Register (IRR) - Where interrupts are first recognized
 * In Service Register (ISR) - Keeps track of which interrupt is being serviced
 * Ineterrupt Mask Register (IMR) - Stores bits which mask the interrupt lines to be masked. IMR operates
 * 				on the IRR. Masking of a higher priority INT
 * 
 * Sequence of events in 8086:
 * 	1. One or more IR lines go high setting the IRR 
 * 	2. 8259A evaluates the interrupts and sends an INT to the CPU if appropriate.
 * 	3. CPU ACKs the interrupt and responds with an (INTA)' pulse
 * 	4. Corresponding bit in the ISR is set, same bit in IRR is reset.
 * 	5. 8086 initiates a second (INTA)' pulse and the 8259A releases 8-bit pointer onto data bus.
 * 	6. Interrupt cycle is complete. The ending sequence differs depending if in AEOI mode or EOI mode:
 * 		a. AEOI Mode - ISR bit is reset
 * 		b. *EOI Mode - ISR bit is reset once the EOI command is issued at the end of the service routine.
 * 
 * Programming  the 8259A:
 * Initialization Command Words (ICWs) - Before normal operation can begin, each 8259A in the system 
 * must be brought to a starting point by a sequence of 2 to 4 Bytes.
 * 
 * Operation Command Words (OCWs) - Command the 8259A to use different algorithms:
 * 	1. Fully nested mode.
 * 	2. Rotating priority mode.
 * 	3. Special mask mode.
 * 	4. Polled mode.
 * These modes can be written into the 8259A at any point after initialization.
 * I did not worry about this.
 * 
 * ICW1: Starts the initialization sequence...
 * 	a. Edge circuit is reset. Following initialization sequence an interrupt must make a low-to-high transition.
 * 	b. IMR is cleared.
 * 	c. IR7 input is assigned priority 7.
 * 	d. Slave mode address is set to 7.
 * 	e. Special Mask Mode is cleared and Status Read (?) is set to IRR.
 * 	f. If IC4 = 0, all functions in ICW4 are set to 0 (Non-Buffered mode, n AEOI). This is not 8086 related?
 * 
 * Bits used (stars indicate what will be used in the 8086:
 * 	Bit 7: Upper bit of interrupt vector address ( not used in 8086 )
 * 	Bit 6: Middle bit of interrupt vector address ( not used in 8086 )
 * 	Bit 5: Lower bit of interrupt vector address ( not used in 8086 )
 * 	Bit 4: Always 1... Starts the initialization sequence.
 * 	Bit 3: LTIM:
 * 		*0 = edge triggered mode
 * 		 1 = level triggered mode
 * 	Bit 2: ADI call address interval 
 * 		*0 = interval of 8 - use bottom 3 bits of 8 bit vector for IRQ number yyyyxxx (y's divisible by 8)
 * 		 1 is interval of 4
 * 	Bit 1: Mode for number of PICs  
 * 		*0 = Cascade mode
 * 		 1 = Single
 * 	Bit 0:  
 * 		 0 = ICW4 not needed.
 * 		*1 = ICW4 needed.
 * 
 * ICW2: Where to remap the interrupt vectors 
 * Master: Start vectors at 0x20
 * 	Bit 7: 0
 * 	Bit 6: 0
 * 	Bit 5: 1
 * 	Bit 4: 0
 * 	Bit 3: 0
 * 	Bit 2: 0
 * 	Bit 1: 0
 * 	Bit 0: 0
 * 
 * Slave: Start vectos are 0x28
 * 	Bit 7: 0
 * 	Bit 6: 0
 * 	Bit 5: 1
 * 	Bit 4: 0
 * 	Bit 3: 1
 * 	Bit 2: 0
 * 	Bit 1: 0
 * 	Bit 0: 0
 * 
 * ICW3: Only hit if we are in cascade mode.
 * When The PICs are cascaded together (done so through a line independent of the IRQ lines), IRQ line 2 
 * must signal the slave PIC. Since each PIC can handle 8 interrupts, we think the cascade could handle
 * 16. BUT, one of the IRQ lines must be used to select for the second PIC. Thus we can handle only 15 total.
 * So what happens if we get an IRQ2? IRQ9 is remapped (automatically to handle IRQ2. IRQ9 should be
 * the first IRQ line of PIC2.
 * 
 * Master: Inform master that there is a slave at IRQ2
 * 	00000100 (0x40)
 * 
 * Slave: Assign ID. Value should coorespond to the value of the line above. 
 * 	00000010 (0x02)
 * 
 * ICW4: Extra information
 * 	Bit 7: 0
 * 	Bit 6: 0
 * 	Bit 5: 0
 * 	Bit 4: SFNM
 * 		*0 = Not special fully nested mode
 * 		 1 = Special fully nested mode
 * 	Bit 3: Buffered Mode High Bit
 * 		*0 = Non buffered
 * 		 1 = Buffered
 * 	Bit 2: Buffered Mode Low Bit ( Don't Care if Bit 3 was 0 )
 * 		 x (0) = Buffered Mode Slave
 * 		 x (1) = Buffered Mode Master
 * 	Bit 1: End of Interrupt
 * 		*0 = Normal EOI
 * 		 1 = Auto EOI
 * 	Bit 0: Overall Mode
 * 		 0 = MCS-80/85 Mode
 * 		*1 = 8086/8088 Mode
*/
static void remapIrq()
{

	unsigned char masterMask = 0;
	unsigned char slaveMask = 0;

	// Save which interrupts we will service (thanks BIOS)
	masterMask = inb(PIC_MASTER_DATA);
	slaveMask = inb(PIC_SLAVE_DATA);	

	// ICW_1: Start initialization sequence and tell the PIC to expect an ICW4
	outb(PIC_MASTER_COMMAND, ICW1_INIT+ICW1_ICW4);
	outb(PIC_SLAVE_COMMAND, ICW1_INIT+ICW1_ICW4);

	// ICW_2: Reprogram interrupt vectors so they don't overlay software exceptions
	outb(PIC_MASTER_DATA, 0x20);
	outb(PIC_SLAVE_DATA, 0x28);

	// ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(PIC_MASTER_DATA, 0x04);

	// ICW3: tell Slave PIC its cascade identity (0000 0010)
	outb(PIC_SLAVE_DATA, 0x02);

	// ICW_4: Set both master and slave into 8086/8088 mode. 
	outb(PIC_MASTER_DATA, ICW4_8086);
	outb(PIC_SLAVE_DATA, ICW4_8086);

	// End the initialization sequence.
	// Is this necessary? 
	outb(PIC_MASTER_DATA, 0x0);
	outb(PIC_SLAVE_DATA, 0x0);

	// Restore masks
	outb(PIC_MASTER_DATA, masterMask);
	outb(PIC_SLAVE_DATA, slaveMask);
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

	remapIrq();

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
 * Function: handleFault
 * Description: Default handler
 *
 *
 */
void handleFault(regs registers, unsigned int errorCode)
{
	printd("handleFault, %p %p\n", registers.eax, errorCode);
	__asm__("hlt");
}

/*
 * Function: doDivideError
 * Exception class: Fault
 * Vector: #0
 *
 * Stack: A dummy error code was pushed for this case in the assembly caller. It is not needed though.
 *
 */
asmlinkage void doDivideError(regs *registers)
{
	printd("doDivideError(): %p, &registers: %p\n", doDivideError, registers);

	// to-do:
	//	1. Kill current
	//	2. Re-schedule
	//	3. We must hlt if we came from ring 0...
	//		call kernel die() routine.

	__asm__("hlt");
		
}

asmlinkage void doDebug(regs *registers)
{
	printd("doDebug(): %p, &registers: %p\n", doDebug, registers);
	__asm__("hlt");
}

asmlinkage void doNmi(regs *registers)
{
	printd("doNmi(): %p, &registers: %p\n", doNmi, registers);
	__asm__("hlt");
}

asmlinkage void doBreakPoint(regs *registers)
{
	printd("doBreakPoint(): %p, &registers: %p\n", doBreakPoint, registers);
	__asm__("hlt");
}

asmlinkage void doOverflow(regs *registers)
{
	printd("doOverflow(): %p, &registers: %p\n", doOverflow, registers);
	__asm__("hlt");
}

asmlinkage void doBoundaryVerification(regs *registers)
{
	printd("doBoundaryVerification(): %p, &registers: %p\n", doBoundaryVerification, registers);
	__asm__("hlt");
}

asmlinkage void doInvalidOpcode(regs *registers)
{
	printd("doInvalidOpcode(): %p, &registers: %p\n", doInvalidOpcode, registers);
	__asm__("hlt");
}

asmlinkage void doDeviceNotAvail(regs *registers)
{
	printd("doDeviceNotAvail(): %p, &registers: %p\n", doDeviceNotAvail, registers);
	__asm__("hlt");
}

asmlinkage void doDoubleFault(regs *registers)
{
	printd("doDoubleFault(): %p, &registers: %p\n", doDoubleFault, registers);
	//dumpBytes(&registers, 128);
	//dumpRegisters(registers);
	// Automatically aborts (hlts)
}

asmlinkage void doCoProcSegOverrun(regs *registers)
{
	printd("doCoProcSegOverrun(): %p, &registers: %p\n", doCoProcSegOverrun, registers);
	__asm__("hlt");
}

asmlinkage void doInvalTss(regs *registers)
{
	printd("doInvalTss(): %p, errorCode: %p, &registers: %p\n", doInvalTss, registers->errorCode, registers);
	__asm__("hlt");
}

asmlinkage void doSegNotPresent(regs *registers)
{
	printd("doSegNotPresent(): %p, errorCode: %p, &registers: %p\n", doSegNotPresent, registers->errorCode, registers);
	__asm__("hlt");
}

// Seg fault
asmlinkage void doStackException(regs *registers)
{
	printd("doStackException(): %p, errorCode: %p, &registers: %p\n", doStackException, registers->errorCode, registers);
	__asm__("hlt");
}

asmlinkage void doGeneralProtection(regs *registers)
{
	printd("doGeneralProtection(): %p, &registers: %p\n", doGeneralProtection, registers);
	//dumpBytes(&x, 128);
	//dumpRegisters(registers);
	//__asm__("hlt");
}

asmlinkage void doPageFault(regs *registers)
{
	printd("doPageFault(): %p, errorCode: %p, &registers: %p\n", doPageFault, registers->errorCode, registers);
	__asm__("hlt");
}

asmlinkage void doFloatError(regs *registers)
{
	printd("doFloatError(): %p, &registers: %p\n", doFloatError, registers);
	__asm__("hlt");
}

asmlinkage void doAlignmentCheck(regs *registers)
{
	printd("doAlignmentCheck(): %p, errorCode: %p, &registers: %p\n", doAlignmentCheck, registers->errorCode, registers);
	__asm__("hlt");
}

asmlinkage void doMachineCheck(regs *registers)
{
	printd("doMachineCheck(): %p, &registers: %p\n", doMachineCheck, registers);
	__asm__("hlt");
}

asmlinkage void doSIMDFloatException(regs *registers)
{
	printd("doSIMDFloatException(): %p, &registers: %p\n", doSIMDFloatException, registers);
	__asm__("hlt");
}

asmlinkage void doVirtException(regs *registers)
{
	printd("doVirtException(): %p, &registers: %p\n", doVirtException, registers);
	__asm__("hlt");
}

asmlinkage void doSystemTimer(regs *registers)
{
	printd("doSystemTimer(): %p, &registers: %p\n", doSystemTimer, registers);

	// Tell the master PIC we are done servicing the interrupt.	
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq1(regs *registers)
{
	printd("doIrq1(): %p, &registers: %p\n", doIrq1, registers);
	
	// Tell the master PIC we are done servicing the interrupt.
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq2(regs *registers)
{
	printd("doIrq2(): %p, &registers: %p\n", doIrq2, registers);

	// Tell the master PIC we are done servicing the interrupt.	
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq3(regs *registers)
{
	printd("doIrq3(): %p, &registers: %p\n", doIrq3, registers);

	// Tell the master PIC we are done servicing the interrupt.
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq4(regs *registers)
{
	printd("doIrq4(): %p, &registers: %p\n", doIrq4, registers);

	// Tell the master PIC we are done servicing the interrupt.
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq5(regs *registers)
{
	printd("doIrq5(): %p, &registers: %p\n", doIrq5, registers);

	// Tell the master PIC we are done servicing the interrupt.	
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq6(regs *registers)
{
	printd("doIrq6(): %p, &registers: %p\n", doIrq6, registers);
	
	// Tell the master PIC we are done servicing the interrupt.
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq7(regs *registers)
{
	printd("doIrq7(): %p, &registers: %p\n", doIrq7, registers);

	// Tell the master PIC we are done servicing the interrupt.	
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq8(regs *registers)
{
	printd("doIrq8(): %p, &registers: %p\n", doIrq8, registers);

	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq9(regs *registers)
{
	printd("doIrq9(): %p, &registers: %p\n", doIrq9, registers);
	
	// Tell master and slave PICs we are finished servicing the interrupt.		
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq10(regs *registers)
{
	printd("doIrq10(): %p, &registers: %p\n", doIrq10, registers);
	
	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq11(regs *registers)
{
	printd("doIrq11(): %p, &registers: %p\n", doIrq11, registers);
	
	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq12(regs *registers)
{
	printd("doIrq12(): %p, &registers: %p\n", doIrq12, registers);

	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq13(regs *registers)
{
	printd("doIrq13(): %p, &registers: %p\n", doIrq13, registers);

	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq14(regs *registers)
{
	printd("doIrq14(): %p, &registers: %p\n", doIrq14, registers);

	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}

asmlinkage void doIrq15(regs *registers)
{
	printd("doIrq15(): %p, &registers: %p\n", doIrq15, registers);

	// Tell master and slave PICs we are finished servicing the interrupt.	
	outb(PIC_SLAVE_COMMAND, 0x20);
	outb(PIC_MASTER_COMMAND, 0x20);
}
	

asmlinkage void doSystemCall(regs *registers)
{
	printd("doSystemCall(): %p, &registers: %p\n", doSystemCall, registers);
	__asm__("hlt");
}

static void dumpRegisters(regs *registers)
{
        printd("gs: %x, ", registers->gs);
        printd("fs: %x, ", registers->fs);
        printd("es: %x, ", registers->es);
        printd("ds: %x\n", registers->ds);
        printd("edi: %p, ", registers->edi);
        printd("esi: %p, ", registers->esi);
        printd("ebp: %p, ", registers->ebp);
        printd("esp: %p\n", registers->esp);
        printd("ebx: %p, ", registers->ebx);
        printd("edx: %p, ", registers->edx);
        printd("ecx: %p, ", registers->ecx);
        printd("eax: %p\n", registers->eax);
	printd("errorCode: %p\n", registers->errorCode);
	printd("eip: %p\n", registers->eip);
	printd("cs: %p, ", registers->cs);
	printd("eflags: %p, ",registers->eflags);
	printd("useresp: %p, ", registers->useresp);
	printd("ss: %p\n", registers->ss);
}

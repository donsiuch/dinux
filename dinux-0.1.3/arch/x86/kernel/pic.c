
#include "x86/inc/pic.h"
#include "x86/inc/system.h"

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
void remapIrq()
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


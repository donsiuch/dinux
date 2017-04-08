
#include "../kernel/include/kernel_defs.h"

.data 
hello: 
	.asciz "Hello world"

// This is currently the main global descriptor table.
.align 16
gdt:
	// Index 0x00
	// Required dummy
	.quad	0x00

	// 0x08
	// Unused
	.quad	0x00

	// 0x10
	// code segment
	// bit 63			bit 32
	// 000000000000000 | 1001101000000000
	// 000000000000000 | 1111111111111111 (limit)
	// bit 31 (base)		bit 0
	.word	0xFFFF
	.word	0x0000
	.word	0x9A00	# 1001 1010 0000 0000
	.word	0x00CF	# 0000 0000 1100 1111
	
	// 0x18
	// data segment
	.word	0xFFFF
	.word	0x0000
	.word	0x9200 # 1001 0010 0000 0000
	.word	0x00CF
gdt_end:
gdt_info:
	.word	gdt_end - gdt - 1	# Size of GDT
	.word	0x0000			# Upper 2 Bytes of GDT address.
	.word	0x0000			# Lower 2 Bytes of GDT address.

idt_info:
	.word	0x0000
	.word	0x0000
	.word	0x0000

.globl setup_32
.text
setup_32:

	call populateIdt

	// Return to real mode
	movw	$0x00,	%ax
	lmsw	%ax
		
	// Fix up idt and ldt info structures
	// Load a new global descriptor table
	movl	$gdt, (gdt_info + 2)	# Store address of GDT
	movl	$idtSize, idt_info
	movl	$idt, (idt_info + 2)
	cli
	lgdt	gdt_info
	lidt	idt_info

	// Return to protected mode
	xorw	%ax, %ax	# Clear register
	movw	$0x01,	%ax
	lmsw	%ax

	sti
	
	// Set the %cs register
	jmp 	$0x10, $loadSegmentRegisters

loadSegmentRegisters:

	// Set all other segments to data registers
	movl	$0x18, %eax
	movl	%eax, %ds
	movl	%eax, %gs
	movl	%eax, %fs
	movl	%eax, %es
	movl	%eax, %ss
	
	call 	kernel_main

	ret

isrSaveState:

	// Clear direction flag.
	cld
	
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl 	%gs
	
	// %eax is populated with the service routine
	call 	*%eax	

	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popa

	iret

.globl placeHolder
placeHolder:
	pusha
	movl 	$handleFault, %eax
	jmp 	$0x10, $isrSaveState

.globl divideError
divideError:
	pusha
	movl 	$doDivideError, %eax
	jmp 	$0x10, $isrSaveState

.globl debug
debug:
	pusha
	movl	$doDebug, %eax
	jmp 	$0x10, $isrSaveState

.globl nmi
nmi:
	pusha
	movl	$doNmi, %eax
	jmp	$0x10, $isrSaveState

.globl breakPoint
breakPoint:
	pusha
	movl	$doBreakPoint, %eax
	jmp	$0x10, $isrSaveState

.globl overflow
overflow:
	pusha
	movl	$doOverflow, %eax
	jmp	$0x10, $isrSaveState

.globl boundaryVerification
boundaryVerifcation:
	pusha
	movl	$doBoundaryVerification, %eax
	jmp	$0x10, $isrSaveState

.globl invalidOpcode
onvalidOpcode:
	pusha
	movl	$doInvalidOpcode, %eax
	jmp	$0x10, $isrSaveState

.globl	deviceNotAvail
deviceNotAvail:
	pusha
	movl	$doDeviceNotAvailIsr, %eax
	jmp	$0x10, $isrSaveState

.globl doubleFault
doubleFault:
	pusha
	movl	$doDoubleFault, %eax
	jmp	$0x10, $isrSaveState

.globl coProcSegOverrun
coProcSegOverrun:
	pusha
	movl	$doCoProcSegOverrun, %eax
	jmp	$0x10, $isrSaveState

.globl invalTss
invalTss:
	pusha
	movl	$doInvalTss, %eax
	jmp	$0x10, $isrSaveState

.globl segNotPresent
segNotPresent:
	pusha
	movl	$doSegNotPresent, %eax
	jmp	$0x10, $isrSaveState

.globl stackException
stackException:
	pusha
	movl	$doStackException, %eax
	jmp	$0x10, $isrSaveState

.globl generalProtection
generalProtection:
	pusha
	movl	$doGeneralProtection, %eax
	jmp	$0x10, $isrSaveState

.globl pageFault
pageFault:
	pusha
	movl	$doPageFault, %eax
	jmp	$0x10, $isrSaveState

.globl floatError
floatError:
	pusha
	movl	$doFloatError, %eax
	jmp	$0x10, $isrSaveState

.globl alignmentCheck
alignmentCheck:
	pusha
	movl	$doAlignmentCheck, %eax
	jmp	$0x10, $isrSaveState

.globl machineCheck
machineCheck:
	pusha
	movl	$doMachineCheck, %eax
	jmp	$0x10, $isrSaveState

.globl systemCall
systemCall:
	pusha
	movl	$doSystemCall, %eax
	jmp	$0x10, $isrSaveState

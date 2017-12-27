
# eax contains magic number 0x2BADB002
# ebx contains Multiboot information structure
# A20 gate is enabled

#define STACK_START 0x9000
#define KERNEL_CS   0x10
#define KERNEL_DS   0x18    
#define PAGING_BIT  0x80000000
#define DUMMY_ERROR_CODE 0x00 

.code32
.text
.globl setup_32
setup_32:

	# Setup a stack
	movl	$STACK_START, %esp
	movl	%esp, %ebp
	
	call populateIdt

	# Fix up idt and ldt info structures
	# Load a new global descriptor table
	#
	# Store address of GDT
	movl	$gdt, (gdt_info + 2)	
	movl	$idtSize, idt_info
	movl	$idt, (idt_info + 2)
	cli
	lgdt	gdt_info
	lidt	idt_info

	# Enable interrupts
	sti	
	
	# Set the %cs register
	jmp 	$KERNEL_CS, $loadSegmentRegisters
loadSegmentRegisters:

	# Set all other segments to data registers
	movl	$KERNEL_DS, %eax
	movl	%eax, %ds
	movl	%eax, %gs
	movl	%eax, %fs
	movl	%eax, %es
	movl	%eax, %ss

	# Turn on paging
	call	setupPaging
	movl	kernel_pd, %eax
	movl	%eax, %cr3
	movl	%cr0, %eax
	orl	    $PAGING_BIT, %eax
	movl	%eax, %cr0 	
	
	call 	kernel_main

	ret

isrSaveState:
	
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl 	%gs

	# Get a pointer to the registers pointer
	movl	%esp, %edx	
	
	# push the register pointer onto the stack
	pushl 	%edx		

	# I read somewhere that clearing the direction
	# flag is the convention.
	cld			

	# %eax is populated with the service routine
	call 	*%eax		
	
	# Pop register pointer from the stack
	addl	$4, %esp		

	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal

	# Pop error code
	addl	$4, %esp	 

	iret

.globl unknownFault
unknownFault:
	pushl	$DUMMY_ERROR_CODE
	pushal
	movl 	$doUnknownFault, %eax
	jmp 	isrSaveState

# 0 	
.globl divideError
divideError:
	# Push dummy error code
	push 	$DUMMY_ERROR_CODE		
	pushal
	movl 	$doDivideError, %eax
	jmp 	isrSaveState

# 1 	
.globl debug
debug:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doDebug, %eax
	jmp 	isrSaveState

# 2
.globl nmi
nmi:
	push 	$DUMMY_ERROR_CODE
	pushal
	movl	$doNmi, %eax
	jmp	    isrSaveState

# 3
.globl breakPoint
breakPoint:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doBreakPoint, %eax
	jmp	    isrSaveState

# 4
.globl overflow
overflow:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doOverflow, %eax
	jmp	    isrSaveState

# 5
.globl boundaryVerification
boundaryVerification:
	cli
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doBoundaryVerification, %eax
	jmp	    isrSaveState

# 6
.globl invalidOpcode
invalidOpcode:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doInvalidOpcode, %eax
	jmp	    isrSaveState

# 7
.globl	deviceNotAvail
deviceNotAvail:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doDeviceNotAvail, %eax
	jmp	    isrSaveState

# 8 
.globl doubleFault
doubleFault:
	pushal
	movl	$doDoubleFault, %eax
	jmp	    isrSaveState

# 9
.globl coProcSegOverrun
coProcSegOverrun:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doCoProcSegOverrun, %eax
	jmp	    isrSaveState

# 10
.globl invalTss
invalTss:
	pushal
	movl	$doInvalTss, %eax
	jmp	    isrSaveState

# 11
.globl segNotPresent
segNotPresent:
	pushal
	movl	$doSegNotPresent, %eax
	jmp	    isrSaveState

# 12
.globl stackException
stackException:
	pushal
	movl	$doStackException, %eax
	jmp	    isrSaveState

# 13
.globl generalProtection
generalProtection:
	pushal
	movl	$doGeneralProtection, %eax
	jmp 	isrSaveState

# 14
.globl pageFault
pageFault:
	pushal
	movl	$doPageFault, %eax
	jmp	    isrSaveState

# 15 ( reserved by intel )

# 16
.globl floatError
floatError:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doFloatError, %eax
	jmp	    isrSaveState

# 17
.globl alignmentCheck
alignmentCheck:
	pushal
	movl	$doAlignmentCheck, %eax
	jmp	    isrSaveState

# 18
.globl machineCheck
machineCheck:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doMachineCheck, %eax
	jmp	    isrSaveState

# 19
.globl simdFloatException
simdFloatException:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doSIMDFloatException, %eax
	jmp	    isrSaveState

# 20
.globl virtException
virtException:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doVirtException, %eax
	jmp	    isrSaveState

# 32 - IRQ 0
.globl systemTimer
systemTimer:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doSystemTimer, %eax
	jmp	    isrSaveState

# 33 - IRQ 1
.globl irq1
irq1:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq1, %eax
	jmp	    isrSaveState

# 34 - IRQ 2
.globl irq2
irq2:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq2, %eax
	jmp	    isrSaveState

# 35 - IRQ 3
.globl irq3
irq3:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq3, %eax
	jmp	    isrSaveState

# 36 - IRQ 4
.globl irq4
irq4:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq4, %eax
	jmp	    isrSaveState

# 37 - IRQ 5
.globl irq5
irq5:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq5, %eax
	jmp	    isrSaveState

# 38 - IRQ 6
.globl irq6
irq6:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq6, %eax
	jmp	    isrSaveState

# 39 - IRQ 7
.globl irq7
irq7:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq7, %eax
	jmp	    isrSaveState

# 40 - IRQ 8
.globl irq8
irq8:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq8, %eax
	jmp	    isrSaveState

# 41 - IRQ 9
.globl irq9
irq9:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq9, %eax
	jmp	    isrSaveState

# 42 - IRQ 10
.globl irq10
irq10:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq10, %eax
	jmp	    isrSaveState

# 43 - IRQ 11
.globl irq11
irq11:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq11, %eax
	jmp	    isrSaveState

# 44 - IRQ 12
.globl irq12
irq12:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq12, %eax
	jmp	    isrSaveState

# 45 - IRQ 13
.globl irq13
irq13:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq13, %eax
	jmp	    isrSaveState

# 46 - IRQ 14
.globl irq14
irq14:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq14, %eax
	jmp	    isrSaveState

# 47 - IRQ 15
.globl irq15
irq15:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doIrq15, %eax
	jmp	    isrSaveState

.globl systemCall
systemCall:
	push	$DUMMY_ERROR_CODE
	pushal
	movl	$doSystemCall, %eax
	jmp	    isrSaveState

.data
string:
	.asciz "--> %p\n"

// This is currently the main global descriptor table.
.align 16
gdt:
	# Index 0x00
	# Required dummy
	.quad	0x00

	# 0x08
	# Unused
	.quad	0x00

	# 0x10
	# code segment
	# bit 63			bit 32
	# 000000000000000 | 1001101000000000
	# 000000000000000 | 1111111111111111 (limit)
	# bit 31 (base)		bit 0
	#.word	0xFFFF
	#.word	0x0000
	#.word	0x9A00	# 1001 1010 0000 0000
	#.word	0x00CF	# 0000 0000 1100 1111
	.quad 0x00cf9a000000ffff	

	# 0x18
	# data segment
	#.word	0xFFFF
	#.word	0x0000
	#.word	0x9200 # 1001 0010 0000 0000
	#.word	0x00CF # 0000 0000 1100 1111 
	.quad 0x00cf92000000ffff	

  # 0x20
	# real mode code segment
	#.word	0xFFFF 	limit (0-15)
	#.word	0x0000	base (16-31)
	#.byte	0x00	base (32-39)
	#.byte	0x9a	access byte (40-47) - 10011010
	#.byte	0x0f	flags + limit (48-55)
	#.word	0x00	base (56-63)
	.quad 0x000f9a000000ffff	

	# 0x28
	# real mode data segment
	.quad 0x000f92000000ffff
gdt_end:
gdt_info:
	.word	gdt_end - gdt - 1	# Size of GDT
	.word	0x0000			# Upper 2 Bytes of GDT address.
	.word	0x0000			# Lower 2 Bytes of GDT address.

idt_info:
	.word	0x0000
	.word	0x0000
	.word	0x0000

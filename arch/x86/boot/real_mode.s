
#
# Filename: real_mode.s
#
# This code to this file is loaded with the kernel and then copied to 0x1000 (thanks to linker script)
#
# This code switches the processor to real mode such that BIOS routines become available.
#
# Donald Siuchninski (December 2017)
#

.section .data.realmode

# IDT for real mode
idt_real_info:
	.word	0x03ff
	.word	0x0000
	.word	0x0000


.code32
.section .text.realmode

.globl switch_to_real_mode
switch_to_real_mode:
    
	pushl   %ebp
   	movl    %esp, %ebp

   	# Load a new code segment with a limit of 0xffff 
   	# This is the segment limit required in real-
   	# address mode.
	jmp	$0x20, $loadRMSeg

.code16
loadRMSeg:
	movl	$0x28, %eax
	movl	%eax, %ds
	movl	%eax, %gs
	movl	%eax, %fs
	movl	%eax, %es
	movl	%eax, %ss

   	lidt	idt_real_info

	# Disable protected mode by disabling PE bit
	movl	%cr0, %eax
   	andb    $0xfe, %al
	movl	%eax, %cr0

	jmp	$0x00, $set_rm_segment_regs
set_rm_segment_regs:
	movw	$0x0000, %ax
	movw	%ax, %ds
	movw	%ax, %gs
	movw	%ax, %fs
	movw	%ax, %es
	movw	%ax, %ss

	# Enable interrupts
	#
	# Note: Interrupts seem to be working whether or 
	# not the interrupt flag is set...
	sti

//    	movb    $0x0e, %ah
//   	movb    $'a', %al
//    	int     $0x10

    # E820 memory test - ask the BIOS for the memory map
    # 
    # Source: http://www.uruk.org/orig-grub/mem64mb.html
    #
    # 
    xorl	%ebx, %ebx
    movw    $0x9000, %di
meme820:	
	movl	$0x0000e820, %eax
	movl 	$0x534D4150, %edx
	movl	$20, %ecx
	int	    $0x15
    
    # Return code checks 
    #
    # Some BIOS's return a 0 carry as an end condition.
    # Carry can also indicate an error. 
    jc      bail820

    # 0 in %ebx can also indicate the ending condition
    cmp     $0, %ebx
    je      bail820
    
    # 'SMAP' should be in %eax, else an error occured
    cmp     $0x534D4150, %eax
    jne     bail820

    # TODO: Add error checking

    # Set the destination of the next 'get'
    addw    $20, %di

    # Run the test again
    jmp     meme820

bail820:

    movl    $0x11041985, (%edi)

	# Disable interrupts
	cli

	# Return to protected mode by setting PE bit
	movl	%cr0, %eax	
	orb	    $0x01, %al
	movl	%eax, %cr0

	# Restore 32 bit code + data segment descriptors
	jmp	$0x10, $load_data_segment_regs
.code32
load_data_segment_regs:
	movl	$0x18, %eax
	movl	%eax, %ds
	movl	%eax, %gs
	movl	%eax, %fs
	movl	%eax, %es
	movl	%eax, %ss

   	movl    %ebp, %esp
   	popl    %ebp
    
	ret


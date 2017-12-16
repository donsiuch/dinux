
.section .data.realmode
real_data:
    .word 0xABBA

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

    # *********************
	# Return to real mode *
	# *********************

    # Load a new segment with a limit of 0xffff 
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
	#movw $0x03ff, (0x600)
    #movw $0x00, (0x602)
    #movw $0x00, (0x604)
	#lidt 0x600

	# Preserve %cr0
	movl	%cr0, %eax

	# Turn off PE bit, bit 0 to return to real mode
	#
	# want to do the following line but can't compile in 16 bit code
    #andl	$0xffffffffe, %eax

    # This is temporary
    andl    $0xfffe, %eax
	movl	%eax, %cr0

	jmp	$0x00, $set_rm_segment_regs
set_rm_segment_regs:
	movw	$0x0000, %ax
	movw	%ax, %ds
	movw	%ax, %gs
	movw	%ax, %fs
	movw	%ax, %es
	movw	%ax, %ss

	# Enable interrupts for memory checking.
#	sti

    movb    $0x0e, %ah
    movb    $'a', %al
    int     $0x10


    # Working
meme820:
	xorl	%ebx, %ebx
	#movw 	$smapBuffer, %di
    movw    $0x9000, %di
    # Pick some address that I anticipate to work
    #movw    $0x500, %es:(%di)
	movl	$0x0000e820, %eax
	movl 	$0x534D4150, %edx
	movl	$20, %ecx
	int	    $0x15
bail820:

#   <code>
    movw    $0xdead, %cx


#	cli

	# **************************
	# Return to protected mode *
	# **************************

	xorw	%ax, %ax
	movw	$0x0001, %ax
	lmsw	%ax

	jmp	$0x10, $restoreGDT
# This directive is required to go here.

.code32
restoreGDT:
	movl	$0x18, %eax
	movl	%eax, %ds
	movl	%eax, %gs
	movl	%eax, %fs
	movl	%eax, %es
	movl	%eax, %ss

    movl    %ebp, %esp
    popl    %ebp
    ret



//#include "../utility/include/io.h"

.data .local
hello: 
	.asciz "Hello world"
true:
	.asciz "True"
false:
	.asciz "False"

/*
	7		   0
	PR Priv 1 Ex DC RW Ac

	7	4
	Gr Sz 0 0

The bit fields are:

    Pr: Present bit. This must be 1 for all valid selectors.
    Privl: Privilege, 2 bits. Contains the ring level, 0 = highest (kernel), 3 = lowest (user applications).
    Ex: Executable bit. If 1 code in this segment can be executed, ie. a code selector. If 0 it is a data selector.
    DC: Direction bit/Conforming bit.
        Direction bit for data selectors: Tells the direction. 0 the segment grows up. 1 the segment grows down, ie. the offset has to be greater than the limit.
        Conforming bit for code selectors:
            If 1 code in this segment can be executed from an equal or lower privilege level. For example, code in ring 3 can far-jump to conforming code in a ring 2 segment. The privl-bits represent the highest privilege level that is allowed to execute the segment. For example, code in ring 0 cannot far-jump to a conforming code segment with privl==0x2, while code in ring 2 and 3 can. Note that the privilege level remains the same, ie. a far-jump form ring 3 to a privl==2-segment remains in ring 3 after the jump.
            If 0 code in this segment can only be executed from the ring set in privl. 
    RW: Readable bit/Writable bit.
        Readable bit for code selectors: Whether read access for this segment is allowed. Write access is never allowed for code segments.
        Writable bit for data selectors: Whether write access for this segment is allowed. Read access is always allowed for data segments. 
    Ac: Accessed bit. Just set to 0. The CPU sets this to 1 when the segment is accessed.
    Gr: Granularity bit. If 0 the limit is in 1 B blocks (byte granularity), if 1 the limit is in 4 KiB blocks (page granularity).
    Sz: Size bit. If 0 the selector defines 16 bit protected mode. If 1 it defines 32 bit protected mode. You can have both 16 bit and 32 bit selectors at once. 

Not shown in the picture is the 'L' bit (bit 21, next to 'Sz') which is used for x86-64 mode. See table 3-1 of the Intel Architecture manual. 	
*/

// This is currently the main global descriptor table.
gdt:
	// Index 0x00
	.quad	0x00

	//0x08
	// code segment
	// bit 63			bit 32
	// 000000000000000 | 1001101000000000
	// 000000000000000 | 1111111111111111 (limit)
	// bit 31 (base)		bit 0
	.word	0xFFFF
	.word	0x0000
	.word	0x9A00	// 1001 1010 0000 0000
	.word	0x00CF	// 0000 0000 1100 1111
	
	// 0x10
	// data segment
	.word	0xFFFF
	.word	0x0000
	.word	0x9200 // 1001 0010 0000 0000
	.word	0x00CF
	
	

.global setup_32
.section .text
setup_32:
	// The next lines clears the terminal. Enable this to print anything.
	call	terminal_initialize
	
	// Don't remember what the next few lines are.
	//movl	%ebx, 	%edx
	//and	$0x100,	%edx
	
	call	ClearTerminal

	// The next three lines are working.
	// For fun, try to do it in assembly.	
	pushl 	$hello
	call 	printd	
	popl	%edx

	//call 	kernel_main 		 
	ret

ClearTerminal:
	push	%ebp
	mov	%esp, %ebp	
	mov	%ds:0xB8000, %edi

	pushl 	$false
	call 	printd	
	popl	%edx

	pop	%ebp
	ret

	

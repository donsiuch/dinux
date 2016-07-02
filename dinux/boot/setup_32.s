
#include "../drivers/vga/include/vga.h"

.data 
hello: 
	.asciz "Hello world"
true:
	.asciz "True"
false:
	.asciz "False"

.global setup_32
.section .text
setup_32:
	call	terminal_initialize

	movl	%ebx, 	%edx
	and	$0x100,	%edx
		

	call 	terminal_writestring
	popl	%edx

	//call 	kernel_main 		 
	ret

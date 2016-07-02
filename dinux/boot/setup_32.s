
#include "../utility/include/io.h"

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

	//movl	%ebx, 	%edx
	//and	$0x100,	%edx
		
	pushl 	$hello
	call 	printd	
	popl	%edx

	//call 	kernel_main 		 
	ret

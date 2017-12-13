# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard. Putting this in a special
# section forces the header to be in the start of the final program.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded.
.section .text
.global _start
.type _start, @function
_start:
	
    jmp setup_32

	# Should never get here	
	cli
	hlt
.Lhang:
	jmp .Lhang


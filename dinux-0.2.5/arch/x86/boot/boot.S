
# File: boot.S

.set ALIGN,    1<<0             # Align loaded modules on page boundaries
.set FLAGS,    ALIGN		# Turn on selected options.
.set MAGIC,    0x1BADB002       # Magic number lets bootloader find this header.
.set CHECKSUM, -(MAGIC + FLAGS) # Checksum of above, to prove we are multiboot.

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Bootloader hands execution to _start
.text
.global _start
.type _start, @function
_start:
	
	jmp setup_32

	# Should never get here	
	cli
	hlt
.Lhang:
	jmp .Lhang


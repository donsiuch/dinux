
OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

__kernel_virtual_start = 0xc0100000;
__physical_load_address = 0x00100000;
__real_mode_code_physical_address = 0x1000;

SECTIONS
{
	/* Set the counter to 0xc0100000 */
	. = __kernel_virtual_start; 

    __kernel_start = .;

	.text ALIGN(4K) : AT(ADDR(.text) - 0xc0000000)
	{
		*(.multiboot);
		*(.text);
	}

	/* Read-only data. */
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xc0000000)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data) - 0xc0000000)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xc0000000) 
	{
		*(COMMON)
		*(.bss)
	}

    __kernel_end = .;

	/* Find the distance between 0x100000 - 0x1000 => 0xff000 */
	__physreal_diff = . - __real_mode_code_physical_address;

	/* AT(ADDR(.realmode)) = 0x1000, (__physreal_diff = 0xc0100000 - 0x1000)
	 	Load at -->  0x1000 + 0xc0100000 - 0x1000 --> 0xc0100000 */
	.realmode __real_mode_code_physical_address : AT(ADDR(.realmode) + __physreal_diff - 0xc0000000) {

		__realmode_vma_start = .;

		/* LOADADDR is the LMA of the specified section */
		__realmode_lma_start = LOADADDR(.realmode);

		*(.text.realmode);
		*(.data.realmode);
	}

	/* Align at 4 Bytes and define some new symbols in our image we can refernce */
	. = ALIGN(4);
	__realmode_vma_end = .;
	__realmode_secsize = ((__realmode_vma_end)-(__realmode_vma_start));

	/* Set virtual address for the rest of the sections beyond the real mode code */
	. += __physreal_diff;

	/* Put other sections the compiler generates here */
}

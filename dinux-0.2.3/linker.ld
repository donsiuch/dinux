
OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

VIRTUAL_OFFSET_ADDRESS = 0xc0100000;
PHYSICAL_BASE_ADDRESS = 0x00100000;
REAL_BASE_ADDRESS = 0x1000;

SECTIONS
{
	/* Set the counter to 0xc0100000 */
	. = VIRTUAL_OFFSET_ADDRESS; 

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

	/* Find the distance between 0x100000 - 0x1000 => 0xff000 */
	__physreal_diff = . - REAL_BASE_ADDRESS;

	/* AT(ADDR(.realmode)) = 0x1000, (__physreal_diff = 0xc0100000 - 0x1000)
	 	Load at -->  0x1000 + 0xc0100000 - 0x1000 --> 0xc0100000 */
	.realmode REAL_BASE_ADDRESS : AT(ADDR(.realmode) + __physreal_diff - 0xc0000000) {

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

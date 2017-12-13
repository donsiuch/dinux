
ENTRY(_start)

SECTIONS
{
	/* Begin putting sections here */
	. = 0x10000;

	/* First put the multiboot header followed by .text section. */
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K) 
	{
		*(COMMON)
		*(.bss)
	}
	
	/* Put other sections the compiler generates here */
}

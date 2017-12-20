
OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

PHYSICAL_BASE_ADDRESS = 0x00100000;
REAL_BASE_ADDRESS = 0x1000;

SECTIONS
{
	/* Begin putting sections here */
	. = PHYSICAL_BASE_ADDRESS;

    /* Find the distance between 0x100000 - 0x1000 => 0xff000 */
    __physreal_diff = . - REAL_BASE_ADDRESS;
    
    /* Tell the linker that the .realmode section should have an addresses */
    /* generated at 0x1000, but is loaded here */
    .realmode REAL_BASE_ADDRESS : AT(ADDR(.realmode) + __physreal_diff) {
    
        *(.multiboot);

        /* The __realmode* values can be used by code to copy
         * the code/data from where it was placed in RAM
         * by the multiboot loader into lower memory at REAL_BASE
         * . (period) is the current VMA */
        __realmode_vma_start = .;

        /* LOADADDR is the LMA of the specified section */
        __realmode_lma_start = LOADADDR(.realmode);
        *(.text.realmode);
        *(.data.realmode);
    }
    
    /* Align at 4 Bytes and define some new symbols in our image we can refernce */
    . = ALIGN(4);
    __realmode_vma_end = .;
    __realmode_secsize   = ((__realmode_vma_end)-(__realmode_vma_start));
    __realmode_secsize_l = __realmode_secsize>>2;
    __realmode_lma_end   = __realmode_vma_start + __physreal_diff + __realmode_secsize;

    . += __physreal_diff;

    .text ALIGN(4K) : AT(ADDR(.text))
	{
		*(.text);
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

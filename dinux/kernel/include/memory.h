#ifndef __MEMORY_HEADER__
#define __MEMORY_HEADER__

// Segment descriptors should be 8 Bytes long.
// Segment descriptor gives all the information that
// is needed about a segment.
//
// A segment descriptor is found in the GDT/LDT. It is chosen
// by the segment selector which is part of the Linear Address.
typedef struct
{
	// Offset into segment of last memory cell
	unsigned short 	limit;

	//---------------------------------------

	// Start address
	unsigned short 	base;

	//---------------------------------------
	
	// ??
	unsigned char	base0;

	// Control bits...
	// code segment or data segment
	unsigned char 	type		:4;
	unsigned char 	sysSeg		:1;	
	unsigned char 	descPrivLevel	:2;
	unsigned char	presentInMem	;1;

	//---------------------------------------

	// ??
	unsigned char 	limit0		:4;

	// control bits
	unsigned char	unused1		:1; //AVL field
	unsigned char	unknown		:1;
	unsigned char	operandSize	:1; // SEE INTEL MANUALS. SIZE RELATED

	// Segment size
	unsigned char	granularity	:1; // 0 == Bytes, 1 == 4096

	unsigned char 	base1;	
} MEM_SEGMENT_DESCRIPTOR;

const unsigned char MEM_MAX_SEG_DESC = 16;
const unsigned char MEM_MAX_CORES = 4;


// The following need to be loaded at an address in an assembly file:
//	1. Must be aligned to an 8 Byte boundary (.align 8)
//	2. Don't know how many tables to allocated. 1x for each core?
//	3. Need an assembly instructions to point the correct GDT entry
//	4. Need to make the GDT array dynamic by querrying the number of cores.
// NUMBER OF CORES IS 4. Detect this dynamically
struct GL_GDT * GL_GDT_ARRAY[GL_MAX_NUM_SEGMENT_DESCRIPTORS];

// The global descriptor table
struct MEM_SEGMENT_DESCRIPTOR GL_GDT[MEM_MAX_CORES];


#endif

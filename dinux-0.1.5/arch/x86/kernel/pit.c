
#include "x86/inc/pit.h"
#include "x86/inc/system.h"
#include "x86/inc/time.h"

/*
 * Thanks James Molloy!
 * 
 * This sets the code to 
*/
void initializePit(uint32_t frequency)
{
	uint32_t divisor	= 0;
	uint8_t lowByte		= 0; 
	uint8_t highByte	= 0;
	_G_TICK			= 0;
	
	// The value we send to the PIT is the value to divide it's input clock
	// (1193180 Hz) by, to get our required frequency. Important to note is
	// that the divisor must be small enough to fit into 16-bits.
	divisor = 1193180 / frequency;

	// Write to command register
	outb(0x43, 0x36);

	// Divisor has to be sent in Bytes, so split here into upper/lower bytes.
	lowByte = (uint8_t)(divisor & 0xFF);
	highByte = (uint8_t)( (divisor>>8) & 0xFF );

	// Send the frequency divisor
	outb(0x40, lowByte);
	outb(0x40, highByte);
}

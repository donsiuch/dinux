
// TO-DO:
// 1. Currently this is tied to the VGA driver. This routine needs to be more generic so I never have to re-write it...
// 2. Change the true and false returns to 0 and 

#include "../include/io.h"
/*
static unsigned char isDigit(const char character)
{
	return 0;
}
static unsigned char isHexDigit(const char character)
{
	if ( character >= 0x00 && character <= 0x0F )
	{
		return 1;
	}
	return 0;
}
*/

// Dumps hexadecimal number
void dumpHex(const unsigned long hexNumber)
{
	int index = 0;
	unsigned char *ptr = 0;
	unsigned char upperNibble = 0;	
	unsigned char lowerNibble = 0;

	terminal_putchar('0');
	terminal_putchar('x');

	while ( index < 4 )
	{
		// Convert little endian into human read-able form
		// Get the last byte, then second to last, etc.
		// Introduce a macro to convert Bytes?
		ptr = (unsigned char*)&hexNumber + ( 4 - (index + 1));
		
		upperNibble = (0xF0 & *ptr) >> 4;
		terminal_putchar( digToAlphaNum(upperNibble+48) );

		lowerNibble = (0x0F & *ptr);
		terminal_putchar( digToAlphaNum(lowerNibble+48));	
	
		index ++;
	}
}

static void vprintd(const char * string, const va_list args)
{
	// While we haven't hit the newline
	while ( *string != 0 )
	{
		char current = *string;

		// If we have a format specifier, the next character specifies how to format it
		if ( current == '%' )
		{
			current = *(string++);

			// Find how we should convert the next argument
			switch (current)
			{
				// decimal
				//case 'd': isDigit('x'); break;
		
				// address
				case 'p': break;
			
				// hex format
				case 'x': break;
	
				// string
				// sub-%'s will be treated as the characters they are
				case 's': break;
					
				default: break; 
			}
		}

		terminal_putchar(current);	
	
		//va_arg(args, #type);	
	
		string++;
	}

	// va_arg is a MACRO that loops through the list.
	// Must pass the type wiht it
	
}

// Prints an entire string
// Changed the name because printf is reserved in gcc
// https://www.gnu.org/software/libc/manual/html_node/Reserved-Names.html
void printd(const char * string, ...)
{
	va_list args;

	va_start(args, string); 

	// Maybe pass to vprintd whatever console to output to?
	vprintd(string, args);		

	va_end(args); 
}

// Convert integer to character and returns that character
char intToChar(const int integer)
{
	return 'a';
}

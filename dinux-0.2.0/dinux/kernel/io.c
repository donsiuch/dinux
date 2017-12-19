
// TO-DO:
// 1. Currently this is tied to the VGA driver. Have a generic API? 

#include "dinux/inc/io.h"

void printHexChar(const unsigned long hexChar)
{
	terminal_putchar( ALPHA_NUM(UPPER_NIBBLE(hexChar)+48) );
	terminal_putchar( ALPHA_NUM(LOWER_NIBBLE(hexChar)+48));	
}

void printFormalHexLong(const unsigned long hexNumber)
{
	int index = 0;
	unsigned char *ptr = 0;

	terminal_putchar('0');
	terminal_putchar('x');

	while ( index < 4 )
	{
		// Get the last byte, then second to last, etc.
		ptr = (unsigned char*)&hexNumber + ( 4 - (index + 1));
		printHexChar(*ptr);	
	
		index ++;
	}
}

static void vprintd(const char *string, va_list args)
{
	// While we haven't hit the newline
	while ( *string != 0 )
	{
		char current = *string;

		// If we have a format specifier, the next character specifies how to format it
		if ( current == '%' )
		{
			current = *(++string);

			// Find how we should convert the next argument
			switch (current)
			{
				// decimal
				//case 'd': isDigit('x'); break;
		
				// address
				case 'p': 
					printFormalHexLong(va_arg(args, const unsigned long));
					break;
			
				// hex character
				case 'x': 
					printHexChar(va_arg(args, const unsigned long));
					break;
	
				// string
				case 's': 
					terminal_writestring(va_arg(args, const char *));
					break;
					
				default: break; 
			}
		}
		else
		{
			terminal_putchar(current);
		}
	
		string++;
	}

	// va_arg is a MACRO that loops through the list.
	// Must pass the type wiht it
	va_end(args);
}

// Prints an entire string
// Changed the name because printf is reserved in gcc
// https://www.gnu.org/software/libc/manual/html_node/Reserved-Names.html
void printd(const char *string, ...)
{
	va_list args;
	va_start(args, string); 
	vprintd(string, args);		
	va_end(args); 
}

/*
 * Function:	void dumpBytes
 *
 * Description: Print size Bytes starting from the address of the
 *		buffer parameter.	
 *
 * Parameters:
 *	buffer	Address to start printing Bytes at	
 *	size	Number of Bytes to print	
 *
 * Returns:	void
 *
*/
void dumpBytes( const unsigned char * buffer, const unsigned long size )
{
	unsigned int index = 0;

	printd("\ndumpBytes @ %p:\n", buffer);

	while ( index < size )
	{
		if ( index % 0x10 == 0 )
		{
			if ( index ) 
			{ 
				printd(" |");
			}
		
			printd("\n| %p: ", buffer+index);
		}
		else if ( index % 0x08 == 0 )
		{
			printd(" ");
		}

		printd("%x", buffer[index]);	

		index ++;
	}
	
	printd(" |\n\n");
}


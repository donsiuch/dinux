
// TO-DO:
// 1. Currently this is tied to the VGA driver. This routine needs to be more generic so I never have to re-write it...
// 2. Change the true and false returns to 0 and 

#include "../include/io.h"

static unsigned char isDigit(const char character)
{
	if ( character >= 0 && character <= 9 )
	{
		return 1;
	}
	return 0;
}

static void vprintd(const char * string, const va_list args)
{
	// While we haven't hit the newline
	while ( *string != 0 )
	{
		// If we have a format specifier, the next character specifies how to format it
		if ( *string == '%' )
		{
			string++;

			// Find how we should convert the next argument
			switch (*string)
			{
				// decimal
				case 'd': isDigit('x'); break;
		
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

		terminal_putchar(*string);	
	
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

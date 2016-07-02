
#include "../include/io.h"

static void vprintd(const char * string, const va_list args)
{
	// va_arg is a MACRO that loops through the list.
	// Must pass the type wiht it
	va_arg(args, #type);
}

// Prints an entire string
// Changed the name because printf is reserved in gcc
// https://www.gnu.org/software/libc/manual/html_node/Reserved-Names.html
void printd(const char * string, ...)
{
	va_list args;

	va_start(args, string); 

	vprintd(string, args);		

	va_end(args); 
}

// Convert integer to character and returns that character
char intToChar(const int integer)
{
	return 'a';
}

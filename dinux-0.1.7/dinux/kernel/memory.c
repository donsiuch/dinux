
#include "dinux/inc/memory.h"

/*
 * Function:	void *memset
 *
 * Description:	Given a memory location, some character and a size,
 * 		write that character to that memory buffer for
 *		that size.
 *
 * Parameters:
 *	s	memory buffer
 *	c	character to write to memory buffer
 *	n	number of characters to write
 *
 * Return:	Returns the buffer parameter address.
 *
*/
void *memset(void *s, int c, unsigned int n)
{
	while ( n-- > 0 )
	{
		*(((unsigned char *)s)+n) = c;
	}
	return s;
}

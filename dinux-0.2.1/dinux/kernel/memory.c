
#include <stdint.h>

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

/*
 * Function:	void *memcpy
 *
 * Description:	Copy source buffer to destination buffer for given size.
 *
 * Parameters:
 *	s	source buffer
 *	c   destination buffer	
 *	n	size to copy
 *
 * Return:	Returns the destination buffer parameter address.
 *
*/
void *memcpy( void *d, void *s, uint32_t n)
{
    while ( n-- > 0)
    {
        *(((unsigned char *)d)+n) = *(((unsigned char *)s)+n);
    }
    return d;
}

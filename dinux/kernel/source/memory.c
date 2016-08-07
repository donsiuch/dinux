
#include "../include/memory.h"

void *memset(void *s, int c, size_t n)
{
	while ( n-- > 0 )
	{
		*(s+n) = c;
	}
	return s;
}

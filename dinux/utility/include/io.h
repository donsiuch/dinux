
/*
 * The IO utility currently implemnts functions to write
 * to the terminal.
 *
 *
 */

#ifndef __IO_HEADER__
#define __IO_HEADER__

#include <stdarg.h>

#include "../../drivers/vga/include/vga.h"

void printHexChar(const unsigned long);
void printFormalHexLong(const unsigned long);
void printd(const char *, ...); 
void dumpBytes(const unsigned char *, const unsigned long);

#define	ALPHA_NUM(_d) (_d >= 0x3A && _d <= 0x3F ? _d + 0x07 : _d)
#define UPPER_NIBBLE(_c) ( (0xF0 & _c) >> 4 )
#define LOWER_NIBBLE(_c) ( (0x0F & _c) )

#endif

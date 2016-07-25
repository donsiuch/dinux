
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

void dumpHex(const unsigned long);
void printd(const char *, ...); 
char intToChar(const int);

#define	digToAlphaNum(d) (d >= 0x3A && d <= 0x3F ? d + 0x07 : d)

#endif


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

void printd(const char *, ...); 
char intToChar(const int);

#endif

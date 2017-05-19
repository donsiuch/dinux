
# include "../include/system.h"

// Read from I/O
unsigned char inb (unsigned short _port)
{
    unsigned char ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (ret) : "dN" (_port));
    return ret;
}

// Write to I/O
void outb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

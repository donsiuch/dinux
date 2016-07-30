CC=gcc
CFLAGS=-Wall -Wextra -pedantic -m32 -nostdlib -std=c99
COMPILE=-c

vga.o: ./source/vga.c ./include/vga.h 
	$(CC) $(CFLAGS) $(COMPILE) ./source/vga.c -I./include -o ./objects/vga.o

clean:
	rm -f ./objects/*.o

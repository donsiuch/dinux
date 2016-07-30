CC=gcc
CFLAGS=-Wall -Wextra -pedantic -m32 -nostdlib -std=c99
COMPILE=-c

all: boot.o setup_32.o

setup_32.o: setup_32.s 
	$(CC) $(CFLAGS) $(COMPILE) setup_32.s -I../kernel/include

boot.o: boot.s
	$(CC) $(CFLAGS) $(COMPILE) boot.s

clean:
	rm -f *.o

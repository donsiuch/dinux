CC=gcc
CFLAGS=-Wall -m32 -nostdlib -std=c99
COMPILE=-c

all: boot.o setup_32.o

setup_32.o: setup_32.s 
	$(CC) $(CFLAGS) $(COMPILE) setup_32.s -I../kernel/include -o setup_32.o

boot.o: boot.s
	$(CC) $(CFLAGS) $(COMPILE) boot.s -o boot.o

clean:
	rm -f *.o

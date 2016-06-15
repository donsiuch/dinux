CC=gcc
CFLAGS=-m32 -nostdlib -std=c99
COMPILE=-c

boot.o: boot.s
	$(CC) $(CFLAGS) $(COMPILE) boot.s

clean:
	rm -f *.o

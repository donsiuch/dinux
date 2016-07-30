CC=gcc
CFLAGS=-Wall -Wextra -pedantic -m32 -nostdlib -std=c99
COMPILE=-c

all: idt.o main.o

main.o: ./source/main.c
	$(CC) $(CFLAGS) $(COMPILE) ./source/main.c -I./include -o ./objects/main.o

idt.o: ./source/idt.c ./include/idt.h
	$(CC) $(CFLAGS) $(COMPILE) ./source/idt.c -I./include -o ./objects/idt.o

clean:
	cd objects && rm -f *.o

CC=gcc
CFLAGS=-Wall -Wextra -pedantic -m32 -nostdlib -std=c99
COMPILE=-c

all: idt system main 

main: ./source/main.c
	$(CC) $(CFLAGS) $(COMPILE) ./source/main.c -I./include -o ./objects/main.o

idt: ./source/idt.c ./include/idt.h
	$(CC) $(CFLAGS) $(COMPILE) ./source/idt.c -I./include -o ./objects/idt.o

system: ./source/system.c ./include/system.h
	$(CC) $(CFLAGS) $(COMPILE) ./source/system.c -I./include -o ./objects/system.o

clean:
	cd objects && rm -f *.o

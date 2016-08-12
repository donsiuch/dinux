CC=gcc
CFLAGS=-Wall -Wextra -pedantic -m32 -nostdlib -std=c99
COMPILE=-c

all: io memoryOperations

io: ./source/io.c ./include/io.h
	$(CC) $(CFLAGS) $(COMPILE) ./source/io.c -I./include -o ./objects/io.o

memoryOperations: ./source/memoryOperations.c ./include/memoryOperations.h
	$(CC) $(CFLAGS) $(COMPILE) ./source/memoryOperations.c -I./include -o ./objects/memoryOperations.o

clean:
	cd objects && rm -f *.o

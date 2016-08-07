CC=gcc
CFLAGS=-Wall -Wextra -pedantic -m32 -nostdlib -std=c99
COMPILE=-c

all: io.o memoryOperations.o

io.o: ./source/io.c ./include/io.h
	$(CC) $(CFLAGS) $(COMPILE) ./source/io.c -I./include -o ./objects/io.o

memoryOperations.o: ./source/memoryOperations.c ./include/memoryOperations.h
	$(CC) $(CFLAGS) $(COMPILE) ./source/memoryOperations.c -I./include -o ./objects/memoryOperations.o

clean:
	cd objects && rm -f *.o

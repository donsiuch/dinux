CC=gcc
CFLAGS=-Wall -Wextra -pedantic -m32 -nostdlib -std=c99
COMPILE=-c

io.o: ./source/io.c ./include/io.h
	$(CC) $(CFLAGS) $(COMPILE) ./source/io.c -I./include -o ./objects/io.o

clean:
	cd objects && rm -f *.o

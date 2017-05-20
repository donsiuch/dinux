
all: io memoryOperations main 

main: main.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) main.c -o ../obj/main.o

io: io.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) io.c -I./include -o ../obj/io.o

memoryOperations: memoryOperations.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) memoryOperations.c -o ../obj/memoryOperations.o

clean:
	rm -f ../obj/*.o

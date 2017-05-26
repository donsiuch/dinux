
all: io memory main 

main: main.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) main.c -o ../obj/main.o

io: io.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) io.c -I./include -o ../obj/io.o

memory: memory.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) memory.c -o ../obj/memory.o

clean:
	rm -f ../obj/*.o

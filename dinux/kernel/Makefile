
all: io memory main mm 

mm: mm.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) mm.c -o ../obj/mm.o

io: io.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) io.c -I./include -o ../obj/io.o

main: main.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) main.c -o ../obj/main.o

memory: memory.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) memory.c -o ../obj/memory.o

clean:
	rm -f ../obj/*.o


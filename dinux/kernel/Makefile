
all: io memory main vmm 

vmm: vmm.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) vmm.c -o ../obj/vmm.o

io: io.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) io.c -I./include -o ../obj/io.o

main: main.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) main.c -o ../obj/main.o

memory: memory.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) memory.c -o ../obj/memory.o

clean:
	rm -f ../obj/*.o


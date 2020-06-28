
all: io memory main vmm math

vmm: vmm.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) vmm.c -o ../obj/vmm.o

io: io.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) io.c -I./include -o ../obj/io.o

main: main.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) main.c -o ../obj/main.o

memory: memory.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) memory.c -o ../obj/memory.o

math: math.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) math.c -o ../obj/math.o

clean:
	rm -f ../obj/*.o


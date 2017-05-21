
all: idt.o pic.o system.o

idt.o: idt.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) idt.c -o ../obj/idt.o 

pic.o: pic.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) pic.c -o ../obj/pic.o

system.o: system.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) system.c -o ../obj/system.o

clean:
	rm -f ../obj/*.o


all: idt.o system.o

idt.o: idt.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) idt.c -o ../obj/idt.o 

system.o: system.c 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) system.c -o ../obj/system.o

clean:
	rm -f ../obj/*.o


all: boot.o setup_32.o

setup_32.o: setup_32.s 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) setup_32.s -o ../obj/setup_32.o 

boot.o: boot.S
	$(CC) $(CFLAGS) $(COMPILE_ONLY) boot.S -o ../obj/boot.o

clean:
	rm -f ../obj/*.o

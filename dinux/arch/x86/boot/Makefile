
all: boot.o setup_32.o

setup_32.o: setup_32.S 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) setup_32.S -o ../obj/setup_32.o 

boot.o: boot.s
	$(CC) $(CFLAGS) $(COMPILE_ONLY) boot.s -o ../obj/boot.o

clean:
	rm -f ../obj/*.o


all: real_mode.o boot.o setup_32.o

real_mode.o: real_mode.s 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) real_mode.s -o ../obj/real_mode.o

setup_32.o: setup_32.S 
	$(CC) $(CFLAGS) $(COMPILE_ONLY) -DASSEMBLY setup_32.S -o ../obj/setup_32.o 

boot.o: boot.S
	$(CC) $(CFLAGS) $(COMPILE_ONLY) boot.S -o ../obj/boot.o

clean:
	rm -f ../obj/*.o

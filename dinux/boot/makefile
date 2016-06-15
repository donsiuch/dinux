CC=gcc
CFLAGS=-m32 -nostdlib -std=c99

kernelExecutable: boot.o kernel.o
	$(CC) $(CFLAGS) boot.s kernel.c -o ../myos.bin


clean:
	rm -f *.o
	rm -f *.bin
	rm -f *.iso

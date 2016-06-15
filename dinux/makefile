
LD=ld
32BIT=-m elf_i386

all: myos.bin

myos.bin: link
	$(LD) $(32BIT) ./boot/boot.o ./kernel/main.o -o myos.bin

link:
	cd ./boot && make
	cd ./kernel && make

clean:
	rm -f *.bin
	rm -f *.iso
	cd ./boot && make clean
	cd ./kernel && make clean	

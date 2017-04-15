
LD=ld
32BIT=-m elf_i386
OPTIONS=-nostartfiles

all: myos.bin

myos.bin: link
	gcc -m32 $(OPTIONS) -o myos.bin \
		./drivers/vga/objects/vga.o \
		./utility/objects/io.o \
		./utility/objects/memoryOperations.o \
		./kernel/objects/main.o \
		./kernel/objects/idt.o \
		./boot/setup_32.o \
		./boot/boot.o \
				
									 

link:
	cd ./boot && make
	cd ./kernel && make
	cd ./drivers/vga && make
	cd ./utility && make

clean:
	rm -f grub.cfg
	rm -rf isodir 
	rm -f *.bin
	rm -f *.iso
	cd ./boot && make clean
	cd ./kernel && make clean	
	cd ./drivers && make clean
	cd ./utility && make clean

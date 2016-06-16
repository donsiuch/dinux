
# For more cool makefile stuff:
# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html

LD=ld
32BIT=-m elf_i386

all: myos.bin

myos.bin: link
	$(LD) $(32BIT) ./boot/boot.o ./kernel/main.o ./drivers/vga/objects/vga.o -o myos.bin

link:
	cd ./boot && make
	cd ./kernel && make
	cd ./drivers/vga && make

clean:
	rm -f grub.cfg
	rm -rf isodir 
	rm -f *.bin
	rm -f *.iso
	cd ./boot && make clean
	cd ./kernel && make clean	
	cd ./drivers && make clean

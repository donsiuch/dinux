
MAKE_DIR	= $(PWD)

ARCH_DIR	:= arch

# Generate this value
MARCH		:= x86

BOOT_DIR	:= $(MAKE_DIR)/$(ARCH_DIR)/$(MARCH)/boot
ARCH_KERNEL_DIR	:= $(MAKE_DIR)/$(ARCH_DIR)/$(MARCH)/kernel
GENERIC_DIR	:= $(MAKE_DIR)/generic
GENERIC_KERNEL_DIR := $(GENERIC_DIR)/kernel
DRIVERS_DIR	:= $(MAKE_DIR)/drivers
VGA_DIR		:= $(DRIVERS_DIR)/vga
VGA_SRC_DIR	:= $(VGA_DIR)/src

ARCH_INCLUDE	:= $(ARCH_DIR)/$(MARCH)/include

INC_SRCH_PATH	:=
INC_SRCH_PATH	+= -I$(ARCH_INCLUDE)
INC_SRCH_PATH	+= -I$(GENERIC_DIR)/include

CC		= gcc
LD		= ld

CFLAGS		:=
CFLAGS		+= $(INC_SRCH_PATH)
CFLAGS		+= -c -m32 -Wall -Wextra -pedantic -std=c11 

LDFLAGS		:=
LDFLAGS		+= -m32 -nostartfiles -T

export MAKE_DIR CC LD CFLAGS LDFLAGS INC_SRCH_PATH

all: myos.bin

myos.bin: objects
	$(CC) $(LDFLAGS) linker.ld -o myos.bin \
	$(VGA_DIR)/obj/vga.o \
	$(GENERIC_DIR)/obj/io.o \
	$(GENERIC_DIR)/obj/memoryOperations.o \
	$(GENERIC_DIR)/obj/main.o \
	$(MAKE_DIR)/arch/x86/obj/system.o \
	$(MAKE_DIR)/arch/x86/obj/idt.o \
	$(MAKE_DIR)/arch/x86/obj/setup_32.o \
	$(MAKE_DIR)/arch/x86/obj/boot.o

objects: 
	@$(MAKE) -C $(BOOT_DIR) -f Makefile
	@$(MAKE) -C $(ARCH_KERNEL_DIR) -f Makefile
	@$(MAKE) -C $(GENERIC_KERNEL_DIR) -f Makefile
	@$(MAKE) -C $(VGA_SRC_DIR) -f Makefile

.PHONY: clean
clean:
	rm -f grub.cfg
	rm -rf isodir 
	rm -f *.bin
	rm -f *.iso
	@$(MAKE) -C $(BOOT_DIR) -f Makefile clean
	@$(MAKE) -C $(ARCH_KERNEL_DIR) -f Makefile clean
	@$(MAKE) -C $(GENERIC_KERNEL_DIR) -f Makefile clean
	@$(MAKE) -C $(VGA_SRC_DIR) -f Makefile clean
	

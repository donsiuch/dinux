
# IF THERE ARE ISSUES BUILDING, DISABLE THE -s FLAG!!!
# Silence all makefile output except errors.
MAKEFLAGS 	+= -s

CC		= gcc
#CC		= ~/x-tools/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-gcc

MAKE_DIR	= $(PWD)

ARCH_DIR	:= arch

# Generate this value
MARCH		:= x86

MARCH_DIR	:= $(MAKE_DIR)/$(ARCH_DIR)/$(MARCH)
BOOT_DIR	:= $(MAKE_DIR)/$(ARCH_DIR)/$(MARCH)/boot
ARCH_KERNEL_DIR	:= $(MAKE_DIR)/$(ARCH_DIR)/$(MARCH)/kernel
DINUX_DIR	:= $(MAKE_DIR)/dinux
DINUX_KERNEL_DIR := $(DINUX_DIR)/kernel
DRIVERS_DIR	:= $(MAKE_DIR)/drivers
VGA_DIR		:= $(DRIVERS_DIR)/vga
VGA_SRC_DIR	:= $(VGA_DIR)/src

ARCH_INCLUDE	:= $(MAKE_DIR)/$(ARCH_DIR)/$(MARCH)/inc
DINUX_INCLUDE	:= $(DINUX_DIR)/inc

# This allows getting rid of absolute (or relative) #include's in the source code files
INC_SRCH_PATH	:=
INC_SRCH_PATH	+= -I$(MAKE_DIR)
INC_SRCH_PATH	+= -I$(MAKE_DIR)/$(ARCH_DIR)
INC_SRCH_PATH	+= -I$(DRIVERS_DIR)

CFLAGS		:=
CFLAGS		+= $(INC_SRCH_PATH)
CFLAGS		+= -c -m32 -Wall -Wextra -pedantic -std=c99 -ffreestanding

# nostdlib is causing issues with the multiboot header
# we want use -nostartfiles in the meantime... but we really want nostdlib
LDFLAGS		:=
LDFLAGS		+= -m32 -O2 -nostartfiles -lgcc -ffreestanding -T

export MAKE_DIR CC LD CFLAGS LDFLAGS INC_SRCH_PATH

all: myos.bin

myos.bin: objects
	$(CC) $(LDFLAGS) linker.ld -o myos.bin \
	$(VGA_DIR)/obj/vga.o \
	$(DINUX_DIR)/obj/io.o \
	$(DINUX_DIR)/obj/memory.o \
	$(DINUX_DIR)/obj/main.o \
	$(MAKE_DIR)/arch/x86/obj/system.o \
	$(MAKE_DIR)/arch/x86/obj/idt.o \
	$(MAKE_DIR)/arch/x86/obj/mm.o \
	$(MAKE_DIR)/arch/x86/obj/time.o \
	$(MAKE_DIR)/arch/x86/obj/pic.o \
	$(MAKE_DIR)/arch/x86/obj/pit.o \
	$(MAKE_DIR)/arch/x86/obj/setup_32.o \
	$(MAKE_DIR)/arch/x86/obj/boot.o 

objects:
	@$(MAKE) -C $(BOOT_DIR) -f Makefile
	@$(MAKE) -C $(ARCH_KERNEL_DIR) -f Makefile
	@$(MAKE) -C $(DINUX_KERNEL_DIR) -f Makefile
	@$(MAKE) -C $(VGA_SRC_DIR) -f Makefile

.PHONY: clean
clean:
	$(RM) -f grub.cfg
	$(RM) -rf isodir 
	$(RM) -f *.bin
	$(RM) -f *.iso
	@$(MAKE) -C $(BOOT_DIR) -f Makefile clean
	@$(MAKE) -C $(ARCH_KERNEL_DIR) -f Makefile clean
	@$(MAKE) -C $(DINUX_KERNEL_DIR) -f Makefile clean
	@$(MAKE) -C $(VGA_SRC_DIR) -f Makefile clean

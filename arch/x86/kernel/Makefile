
all: arch_mm idt  meme820 pic pit time system buddy

arch_mm: arch_mm.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) arch_mm.c -o ../obj/arch_mm.o

buddy: buddy.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) buddy.c -o ../obj/buddy.o

idt: idt.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) idt.c -o ../obj/idt.o

meme820: meme820.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) meme820.c -o ../obj/meme820.o

pic: pic.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) pic.c -o ../obj/pic.o

pit: pit.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) pit.c -o ../obj/pit.o

time: time.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) time.c -o ../obj/time.o

system: system.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) system.c -o ../obj/system.o

clean:
	rm -f ../obj/*.o


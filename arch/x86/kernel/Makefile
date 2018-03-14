
all: idt.o  meme820.o mm.o pic.o pit.o time.o system.o

idt.o: idt.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) idt.c -o ../obj/idt.o

meme820.o: meme820.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) meme820.c -o ../obj/meme820.o

mm.o: mm.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) mm.c -o ../obj/mm.o

pic.o: pic.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) pic.c -o ../obj/pic.o

pit.o: pit.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) pit.c -o ../obj/pit.o

time.o: time.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) time.c -o ../obj/time.o

system.o: system.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) system.c -o ../obj/system.o

clean:
	rm -f ../obj/*.o

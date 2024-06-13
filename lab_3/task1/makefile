link: start util
	ld -m elf_i386 start.o util.o -o start

util: util.c util.h
	gcc -g -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

start: start.s
	nasm -g -f elf32 start.s -o start.o

clean:
	rm -f *.o start

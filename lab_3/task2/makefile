link: start.o main.o util.o
	ld -m elf_i386 start.o main.o util.o -o start

start.o: start.s
	nasm -g -f elf32 start.s -o start.o

main.o: main.c
	gcc -m32 -Wall -ansi -g -c -nostdlib -fno-stack-protector main.c -o main.o

util.o: util.c util.h
	gcc -m32 -Wall -ansi -g -c -nostdlib -fno-stack-protector util.c -o util.o

clean:
	rm -f *.o start

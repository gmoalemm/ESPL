hello: hello_world.s
	nasm -f elf32 hello_world.s -o hello_world.o
	ld -m elf_i386 hello_world.o -o hello_world


link: start main util
	ld -m elf_i386 start.o main.o util.o -o task0

start: start.s
	nasm -f elf32 start.s -o start.o

main: main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

util: util.c util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

clean:
	rm -f *.o task0 hello_world

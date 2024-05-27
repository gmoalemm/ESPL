all: sum

sum: bin/main.o bin/numbers.o bin/add.o
	gcc -m32 -Wall -o sum bin/main.o bin/numbers.o bin/add.o

bin/main.o:	src/main.c
	gcc -m32 -Wall -c -o bin/main.o src/main.c

bin/numbers.o: src/numbers.c
	gcc -m32 -Wall -c -o bin/numbers.o src/numbers.c

bin/add.o: src/add.s
	nasm -f elf -w+all -o bin/add.o src/add.s

clean:
	rm -f bin/* sum

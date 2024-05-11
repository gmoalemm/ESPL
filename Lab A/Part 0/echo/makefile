all: my_echo

my_echo: bin/main.o
	gcc -m32 -Wall -o my_echo bin/main.o

bin/main.o:	src/main.c
	gcc -m32 -Wall -c -o bin/main.o src/main.c

clean:
	rm -f bin/* my_echo

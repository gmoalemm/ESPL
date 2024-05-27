all: addresses menu

addresses: bin/addresses.o
	gcc -m32 -Wall -o addresses bin/addresses.o

bin/addresses.o: src/addresses.c
	gcc -m32 -Wall -c -o bin/addresses.o src/addresses.c

menu: bin/menu.o
	gcc -m32 -Wall -o menu bin/menu.o

bin/menu.o: src/menu.c
	gcc -m32 -Wall -c -o bin/menu.o src/menu.c

clean:
	rm -f addresses menu bin/*.o

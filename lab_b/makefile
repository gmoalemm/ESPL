all: bubblesort

runval: bubblesort
	valgrind --leak-check=full ./bubblesort 1 2 4 3

bubblesort: bin/bubblesort.o
	gcc -m32 -Wall -g -o bubblesort bin/bubblesort.o

bin/bubblesort.o: src/bubblesort.c
	gcc -m32 -Wall -g -c -o bin/bubblesort.o src/bubblesort.c

clean:
	rm -f bin/* bubblesort

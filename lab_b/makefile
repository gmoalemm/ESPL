# ! change the directories when submitting

all: part0 part1

part1: virusDetector

virus_val: virusDetector
	valgrind --leak-check=full ./virusDetector

virusDetector: bin/virusDetector.o
	gcc -m32 -Wall -g -o virusDetector bin/virusDetector.o

bin/virusDetector.o: src/virusDetector.c
	gcc -m32 -Wall -g -c -o bin/virusDetector.o src/virusDetector.c

part0: bubblesort hexaPrint

hexaPrint: bin/hexaPrint.o
	gcc -m32 -Wall -g -o hexaPrint bin/hexaPrint.o

bin/hexaPrint.o: src/hexaPrint.c
	gcc -m32 -Wall -g -c -o bin/hexaPrint.o src/hexaPrint.c

bubble_val: bubblesort
	valgrind --leak-check=full ./bubblesort 1 2 4 3

bubblesort: bin/bubblesort.o
	gcc -m32 -Wall -g -o bubblesort bin/bubblesort.o

bin/bubblesort.o: src/bubblesort.c
	gcc -m32 -Wall -g -c -o bin/bubblesort.o src/bubblesort.c

clean:
	rm -f bin/* bubblesort hexaPrint virusDetector

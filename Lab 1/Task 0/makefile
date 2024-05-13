count: count.o
	gcc -m32 -Wall -o count count.o

count.o: count-words.c
	gcc -m32 -Wall -g -c -o count.o count-words.c

clean:
	rm -f count count.o

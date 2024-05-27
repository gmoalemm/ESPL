all: myshell

myshell: myshell.o LineParser.o
	gcc -m32 -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c
	gcc -m32 -Wall -g -c -o myshell.o myshell.c

LineParser.o: LineParser.h LineParser.c
	gcc -m32 -Wall -g -c -o LineParser.o LineParser.c

clean:
	rm -f myshell myshell.o LineParser.o

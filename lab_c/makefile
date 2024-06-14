all: task1 task2

task2: myshell

task1: mypipeline

mypipeline: mypipeline.o
	gcc -m32 -Wall -o mypipeline mypipeline.o

mypipeline.o: mypipeline.c
	gcc -m32 -Wall -g -c -o mypipeline.o mypipeline.c

myshell: myshell.o LineParser.o
	gcc -m32 -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c
	gcc -m32 -Wall -g -c -o myshell.o myshell.c

LineParser.o: LineParser.h LineParser.c
	gcc -m32 -Wall -g -c -o LineParser.o LineParser.c

clear:
	rm -f mypipeline mypipeline.o myshell myshell.o LineParser.o
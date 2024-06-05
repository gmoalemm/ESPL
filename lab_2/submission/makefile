all: myshell looper mypipe

mypipe: mypipe.o
	gcc -m32 -Wall -o mypipe mypipe.o

mypipe.o: mypipe.c
	gcc -m32 -Wall -g -c -o mypipe.o mypipe.c

looper: looper.o
	gcc -m32 -Wall -o looper looper.o

looper.o: looper.c
	gcc -m32 -Wall -g -c -o looper.o looper.c

myshell: myshell.o LineParser.o
	gcc -m32 -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c
	gcc -m32 -Wall -g -c -o myshell.o myshell.c

LineParser.o: LineParser.h LineParser.c
	gcc -m32 -Wall -g -c -o LineParser.o LineParser.c

clean:
	rm -f myshell myshell looper mypipe *.o

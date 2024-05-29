all: myshell looper mypipe

mypipe: bin/mypipe.o
	gcc -m32 -Wall -o mypipe bin/mypipe.o

bin/mypipe.o: src/mypipe.c
	gcc -m32 -Wall -g -c -o bin/mypipe.o src/mypipe.c

looper: bin/looper.o
	gcc -m32 -Wall -o looper bin/looper.o

bin/looper.o: src/looper.c
	gcc -m32 -Wall -g -c -o bin/looper.o src/looper.c

myshell: bin/myshell.o bin/LineParser.o
	gcc -m32 -Wall -o myshell bin/myshell.o bin/LineParser.o

bin/myshell.o: src/myshell.c
	gcc -m32 -Wall -g -c -o bin/myshell.o src/myshell.c

bin/LineParser.o: include/LineParser.h src/LineParser.c
	gcc -m32 -Wall -g -c -o bin/LineParser.o src/LineParser.c

clean:
	rm -f myshell bin/* myshell looper mypipe

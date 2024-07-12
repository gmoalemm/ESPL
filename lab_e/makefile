myELF: myELF.o
	gcc -m32 -g -o myELF myELF.o

myELF.o: myELF.c
	gcc -m32 -g -c myELF.c -o myELF.o

clean:
	rm -f myELF myELF.o

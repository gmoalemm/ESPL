all:
	gcc -Wall -g -m32 -no-pie ./task1.c -o hexeditplus
	gcc -Wall -fno-pie -fno-stack-protector -g -m32 ./task4.c -o task4

.PHONY: clean
    
clean:
	rm -f ./hexeditplus ./task4
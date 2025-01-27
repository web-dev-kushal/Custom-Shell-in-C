test:
	gcc -c shell.c 
	gcc -g -o shell shell.o

	./shell
.PHONY: all clean




all: shell


shell: shell.o llist.o
	gcc -o myshell shell.o llist.o



shell.o: shell.c llist.h
	gcc -c shell.c



llist.o: llist.c
	gcc -c llist.c



clean:
	rm -rf *.o myshell
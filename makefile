.PHONY: all clean




all: myshell


myshell: myshell.o llist.o
	gcc -o myshell myshell.o llist.o



myshell.o: myshell.c llist.h
	gcc -c myshell.c



llist.o: llist.c
	gcc -c llist.c



clean:
	rm -rf *.o myshell

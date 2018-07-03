CC       = gcc
CCFLAGS  = -Wall -g

tryme: tryme.o
	gcc -L //home/njspence/CPE453/Projects/Project1/lib64 -o tryme tryme.o -lmalloc

tryme.o: tryme.c
	gcc -Wall -g -c -o tryme.o tryme.c

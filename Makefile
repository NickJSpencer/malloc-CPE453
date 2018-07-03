CC = gcc
CFLAGS = -Wall -g -fpic

intel-all: lib/libmalloc.so lib64/libmalloc.so

clean:
	rm -f *.o *~ TAGS

lib/libmalloc.so: lib malloc32.o Header32.o
	$(CC) $(CFLAGS) -m32 -shared -o $@ malloc32.o Header32.o

lib64/libmalloc.so: lib64 malloc64.o Header64.o
	$(CC) $(CFLAGS) -m64 -shared -o $@ malloc64.o Header64.o

lib:
	mkdir lib

lib64:
	mkdir lib64

malloc32.o: malloc.c 
	$(CC) $(CFLAGS) -m32 -c -o malloc32.o malloc.c

malloc64.o: malloc.c
	$(CC) $(CFLAGS) -m64 -c -o malloc64.o malloc.c
 
Header32.o: Header.c
	$(CC) $(CFLAGS) -m32 -c -o Header32.o Header.c

Header64.o: Header.c
	$(CC) $(CFLAGS) -m64 -c -o Header64.o Header.c


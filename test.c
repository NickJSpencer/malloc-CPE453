#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include <stdint.h>
#include "Header.h"

int main(int argc, char *argv[]) 
{
   char* size;
   char* test = "hello world\n";
   write(1, test, strlen(test));
   sprintf(size, "%d", sizeof(Header));
   write(1, size, 8);
   
   int* test1 = (int*) malloc(sizeof(int));
   
   return 0;
}
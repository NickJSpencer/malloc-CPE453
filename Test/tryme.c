#include<string.h>
#include<stdlib.h>
#include<stdio.h>

int main(int argc, char *argv[]) 
{
   int* a;
   char* b;
   char* c;
   a = (int *) calloc(16, sizeof(int));
   b = (char*) malloc(16);
   b = (char*) realloc(b, 32);
   //b = (char *) malloc(160000);
   //*a = 34;
   //c = (char *) malloc(16);
   //b = "there";
   //c = "sdkjfn";
   //puts(a);
   //puts(b);
   //puts(c);
   free(a);
   //a = "yo";
   //b = "bro";
   //puts(a);
   //puts(b);
   
/*    char *s;
   s = strdup("Tryme"); //should call malloc() implicitly
   puts(s);
   free(s); */
   //int* test = (int*) malloc(sizeof(int));
   //test = (int*) malloc(sizeof(int));
   //test = 4;
   return 0;
}
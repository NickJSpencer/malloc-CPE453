#ifndef HEADER_H
   #define HEADER_H
   #include <stdbool.h>
   #include <stdint.h>
   #include <stddef.h>

   typedef struct Header
   {
      struct Header* next;
      struct Header* prev;
      uintptr_t data;
      bool isFree;
      uintptr_t size;
   } Header;
   
   Header make_header(uintptr_t size, Header* prev, 
      uintptr_t location, bool isFree);
   
#endif
#include "Header.h"
#define HEADER_SIZE sizeof(Header) + sizeof(Header) % 16

Header make_header(uintptr_t size, Header* prev, 
   uintptr_t location, bool isFree)
{
   Header header;
   header.next = NULL;
   header.prev = &(*prev);
   header.data = (uintptr_t) location + HEADER_SIZE;
   header.isFree = isFree;
   header.size = (int) size;
   return header;
}
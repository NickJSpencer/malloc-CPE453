#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "Header.h"

#define TRUE 1
#define FALSE 0
#define CHUNK 65536
#define DIVISION 16
#define HEADER_SIZE (sizeof(Header) + sizeof(Header) % 16)

#define STDERR 2

Header* head_ptr;
Header head;

Header* malloc_helper(uintptr_t size);

Header* get_first_header();
Header* get_my_header(Header* header_ptr, uintptr_t size);

Header* internal_pointer_get_header(Header* header_ptr, uintptr_t ptr);
void merge_headers(Header *header_ptr);

void allocate_memory(Header* header_ptr, uintptr_t size);

void write_realloc_debug(void* ptr, uintptr_t size,
   Header* my_header_ptr, uintptr_t real_size);
   
uintptr_t get_real_size(uintptr_t size);
uintptr_t allocate_new_chunk();

/* The main calloc function */
void *calloc(uintptr_t nmemb, uintptr_t size)
{
   uintptr_t total_size = nmemb * size;
   uintptr_t real_size = total_size;
   void* malloc_result;
   Header* my_header_ptr;
   
   // Get the real size that is divisible by 16
   real_size = get_real_size(total_size);
   
   // call main malloc helper function
   my_header_ptr = malloc_helper(real_size);
   
   // if DEBUG_MALLOC env var is enabled
   if (getenv("DEBUG_MALLOC") != NULL)
   {
      char string[64];
      snprintf(string, 64, "MALLOC: calloc(%d,%d)   =>  (ptr=%p, size=%d)\n",
         nmemb, size, my_header_ptr->data, real_size);
      fputs(string, stderr);
   }
   
   // return the pointer to the data
   return (void*) my_header_ptr->data;
}

/* Main malloc function */
void *malloc(uintptr_t size)
{
   // Get the real size that is divisible by 16
   uintptr_t real_size = get_real_size(size);
   Header* my_head_ptr;
   Header* my_header_ptr;
   
   // call main malloc helper function
   my_header_ptr = malloc_helper(real_size);
   
   // if DEBUG_MALLOC env var is enabled
   if (getenv("DEBUG_MALLOC") != NULL)
   {
      char string[64];
      snprintf(string, 64, "MALLOC: malloc(%d)      =>  (ptr=%p, size=%d)\n",
         size, my_header_ptr->data, real_size);
      fputs(string, stderr);
   }
   
   // return the pointer to the data
   return (void*) my_header_ptr->data;
}

/* Main malloc helper function that goes through LL to find 
   appropriate header that will store new data*/
Header* malloc_helper(uintptr_t size)
{
   // Get the head of the LL
   Header* my_head_ptr = get_first_header();
   
   // Find the appropriate header for the new data
   Header* my_header_ptr = get_my_header(my_head_ptr, size);
   
   // Creat the space for the new data and create a "next" header 
   allocate_memory(my_header_ptr, size);
   
   return my_header_ptr;
}

/* Gets the head of the LL */
Header* get_first_header()
{
   // If the head of the LL is not yet created
   if (head_ptr == NULL)
   {
      // Create location for the head
      uintptr_t location = allocate_new_chunk();
      
      head_ptr = (Header *) location;
      *(head_ptr) = make_header(CHUNK - HEADER_SIZE, NULL, location, TRUE);
      
      return head_ptr;
   }
   // If the head is already created (most of the time)
   else
   {
      return head_ptr;
   }
}

/* Finds the appropriate header for the new data */
Header* get_my_header(Header* header_ptr, uintptr_t size)
{
   // if there is the header is free and there is room for the allocation
   if (header_ptr->isFree && header_ptr->size >= size)
   {
      return header_ptr;
   }
   // if there is a next header in the LL
   else if (header_ptr->next != NULL)
   {
      return get_my_header(header_ptr->next, size);
   }
   // if this is the last header in the LL
   else
   {
      //Header new_header = create_new_header()
      if (header_ptr->isFree)
      {
         // not enough room for allocation
         allocate_new_chunk();
         header_ptr->size += CHUNK;

         return get_my_header(header_ptr, size);
      }
      // this should never get here; there should always be a free header
      return NULL;
   }
}

/* Allocates the correct amount of memory for the new data */
void allocate_memory(Header* header_ptr, uintptr_t size)
{
   uintptr_t remaining_space;
   
   if (header_ptr->next == NULL) // If header is at end of the LL
   {
      // finds remaining space between last header and the brk
      remaining_space = (uintptr_t)sbrk(0) - (uintptr_t)header_ptr - 
         HEADER_SIZE - size;
         
      // if there is more space for another header
      if (remaining_space >= HEADER_SIZE + DIVISION)
      {
         // create a new header as this header's "next"
         uintptr_t location = ((uintptr_t) header_ptr) + HEADER_SIZE + size;
         
         header_ptr->next = location;
         *(header_ptr->next) = make_header(remaining_space - HEADER_SIZE, 
            header_ptr, location, TRUE);
         header_ptr->isFree = FALSE;
         header_ptr->size = size;
      }
      else // If there is not enough space for another header
      {
         allocate_new_chunk();
         allocate_memory(header_ptr, size);
      }
   }
   else // If header is not the end of LL
   {
      // finds remaining space between this header and the next header
      remaining_space = (uintptr_t) header_ptr->next - 
         HEADER_SIZE - size;
         
      // if there is more space for another header
      if (remaining_space >= HEADER_SIZE + DIVISION)
      {
         // create a new header as this header's "next"
         uintptr_t location = ((uintptr_t) header_ptr) + HEADER_SIZE + size;
         header_ptr->next = location;
         
         *(header_ptr->next) = make_header(remaining_space - HEADER_SIZE, 
            header_ptr, location, TRUE);
         header_ptr->isFree = FALSE;
         header_ptr->size = size;
      }
      else // If there is not enough space for another header
      {
         header_ptr->isFree = FALSE;
         header_ptr->size = size;
      }
   }
}

/* Main free function */
void free(void *ptr)
{
   // if the free function is being called on a real pointer
   if (ptr != NULL)
   {
      uintptr_t data = (uintptr_t) ptr;
      Header* my_header_ptr = internal_pointer_get_header(head_ptr, 
         (uintptr_t) ptr);
      
      // if the pointer does not point to any of the data allocated
      if (my_header_ptr == NULL)
      {
         return;
      }
      
      // free the data
      my_header_ptr->isFree = TRUE;
      
      // merge with any surrounding headers that are free
      merge_headers(my_header_ptr);
      
      // if DEBUG_MALLOC env var is enabled
      if (getenv("DEBUG_MALLOC") != NULL)
      {
         char string[64];
         snprintf(string, 64, "MALLOC: free(%p)\n", ptr);
         fputs(string, stderr);
      }
   }
}

/* Get the header associated with the pointer, even if its not pointing 
   to the start of the data */
Header* internal_pointer_get_header(Header* header_ptr, uintptr_t ptr)
{
   Header* next_header_ptr;
   uintptr_t next_ptr;
   uintptr_t this_ptr;
   
   // if the header does not exist, there is nothing more to do
   if (header_ptr == NULL)
   {
      return NULL;
   }
   
   next_header_ptr = header_ptr->next;
   next_ptr = (uintptr_t) next_header_ptr;
   this_ptr = (uintptr_t) header_ptr;
   
   // if the pointer is within this header, return it
   if (ptr >= this_ptr && ptr < next_ptr)
   {
      return header_ptr;
   }
   
   // iterate to the next header in the LL
   return internal_pointer_get_header(next_header_ptr, ptr);
}

/* Merge a free header with surrounding free headers */
void merge_headers(Header *header_ptr)
{
   Header* next_ptr = header_ptr->next;
   Header* prev_ptr = header_ptr->prev;
   
   // if there is a next header
   if(next_ptr != NULL)
   {
      // if the next header is free
      if (next_ptr->isFree)
      {
         //merge with next
         header_ptr->size += next_ptr->size + HEADER_SIZE;
         header_ptr->next = next_ptr->next;
         merge_headers(header_ptr);
      }
   }
   // if there is a prev header
   if(prev_ptr != NULL)
   {
      // if the prev header is free
      if (prev_ptr->isFree)
      {
         //merge with prev by recursive call on prev
         merge_headers(prev_ptr);
      }
   }
}

/* The main realloc function */
void *realloc(void *ptr, uintptr_t size)
{
   Header* my_header_ptr;
   uintptr_t data = (uintptr_t) ptr;
   uintptr_t real_size = get_real_size(size);
   
   // if the pointer points to nothing
   if (ptr == NULL)
   {
      // do the same thing as a malloc call
      my_header_ptr = malloc_helper(real_size);
      write_realloc_debug(ptr, size, my_header_ptr, real_size);
      return (void *) my_header_ptr->data;
   }
   
   // get the correct header pointer is pointing to
   my_header_ptr = internal_pointer_get_header(head_ptr, (uintptr_t) ptr);
   
   // if it cannot find a header, return nothing
   if (my_header_ptr == NULL)
   {
      return (void*) 0;
   }
   
   // if the size is 0
   if (size == 0)
   {
      // do the same thing as free
      free(ptr);
      write_realloc_debug(ptr, size, my_header_ptr, real_size);
      return (void *) NULL;
   }
   
   // merge surrounding free headers
   merge_headers(my_header_ptr);
   
   // if there is not enough room in the current header
   if (my_header_ptr->size < real_size)
   {
      // do the same thing as malloc
      free(ptr);
      my_header_ptr = malloc_helper(real_size);
   }
   // if there is enough room in the current header
   else
   {
      // allocate the memory
      allocate_memory(my_header_ptr, real_size);
   }
      
   write_realloc_debug(ptr, size, my_header_ptr, real_size);
   
   return (void*) my_header_ptr;
}

/* Writes if the DEBUG_MALLOC env var is indicated */
void write_realloc_debug(void* ptr, uintptr_t size,
   Header* my_header_ptr, uintptr_t real_size)
{
   if (getenv("DEBUG_MALLOC") != NULL)
   {
      char string[64];
      snprintf(string, 64, "MALLOC: realloc(%p,%d)  =>  (ptr=%p, size=%d)\n",
         ptr, size, my_header_ptr->data, real_size);
      fputs(string, stderr);
   }
}

/* Get the real size that is divisible by 16 */
uintptr_t get_real_size(uintptr_t size)
{
   if (size % DIVISION != 0)
   {
      return (uintptr_t) (size + DIVISION - size % DIVISION);
   }
   return size;
}

uintptr_t allocate_new_chunk()
{
   return sbrk(CHUNK);
}
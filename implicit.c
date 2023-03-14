/* CS 107
Code written by: Christo Hristov

This file contains a series of utility functions implemented to allocate, free, and reallocate memory from a heap.  These functions are used in the test_implicit.c file.
*/

#include "./allocator.h"
#include "./debug_break.h"
#include <stdio.h>
#include <string.h>

#define HEADER_SIZE 8  // define a constant to hold the number of bytes in a header
static void *segment_start;
static size_t segment_size;

// create a struct, header, to hold the size the block of memory indicated by the header
typedef struct {
    size_t size;  // number ending in one if the header is used and zero if header is free
} header;

/* Function: roundup
--------------------------
Given an amount, sz, and a mulitplier, mult, roundup returns the smallest number that is a multiple of mult and is larger or equal to sz.
*/

size_t roundup(size_t sz, size_t mult) {
    return (sz + mult - 1) & ~(mult - 1);
}

/* Function: is_free
---------------------------
Given a void pointer, headerptr, is_free returns true if headerptr points to a header that indicates a free block, and false if headerptr points to a header that indicates a used block.

This function assumes that headerptr points to a header in the heap memory payload.
*/

bool is_free(void *headerptr) {
    header *header_ptr = (header *)headerptr;
    // checking if least significant digit is 0 or 1
    if ((header_ptr->size & 1) == 0) {
        return true;
    } else {
        return false;
    }
}

/* Function: make_used
-----------------------------
Given a void pointer, headerptr, and a requested size, requested_size, make_used will change the header pointed to by headerptr to indicate a used block in memory with requested_size bytes.

This function assumes that headerptr points to a header in the heap.
*/

void make_used(void *headerptr, size_t requested_size) {
    header *header_ptr = (header *)headerptr;
    header_ptr->size = requested_size + 1;  // add one to the end to indicate used block
}

/* Function: make_free
-----------------------------
Given a void pointer, headerptr, and a requested size, space, make_free will change the header pointed to by headerptr to indicate a free block in memory with space bytes.

This function assumes that headerptr points to a header in the heap and that space is an alligned amount.
*/

void make_free(void *ptr, size_t space) {
    header *header_ptr = (header *)ptr;
    header_ptr->size = space;
}

/* Function: myinit
----------------------------
Given a void pointer to the start of the heap, heap_start, and the size of the heap, heap_size, myinit intializes heap_size bytes of memory starting at heap_start to be used as the heap.  This is done by updating the global variables that refer to the size and start of the heap.  Subsequent calls to myinit will clear the current heap and reiniatalize a new heap with the inputted paramaters.  myinit will return false if the inputted size is too small for the heap allocated to use, and otherwise will return true.

This function assumes that heap_start is a non null pointer that is alligned with the ALLIGNMENT constant, and that heap size is a mulitple of ALLIGNMENT constant.
*/

bool myinit(void *heap_start, size_t heap_size) {
    // if the heap_size is less than eight, there is not enough memory to be allocated
    if (heap_size <= HEADER_SIZE) {
        return false;
    }
    segment_start = heap_start;  
    segment_size = heap_size;
    header *first_header = (header *)heap_start;
    first_header->size = heap_size - HEADER_SIZE;  // intializing a header that indicates the whole heap is free to use
    return true;
}

/* Function: mymalloc
------------------------------------
Given a number of bytes, requested_size, mymalloc will return a pointer to an address in the heap that contains an alligned requested_size number of bytes to be used by the caller.  If requested_size is 0 or there is not enough free memory in the heap to accomodate the user's request, mymalloc will return a NULL pointer.
*/

void *mymalloc(size_t requested_size) {
    void *result = NULL;
    // if the input is 0
    if (requested_size == 0) {
        return result;
    }
    size_t needed = roundup(requested_size, ALIGNMENT);  // calcuate the alligned number of bytes needed
    void *temp = segment_start;  // create a temporary pointer to traverse the heap
    void *end_heap =  (char *)segment_start + segment_size;  // intialize a pointer to the end of the heap
    // while there are still headers left to check
    while (temp < end_heap) {
        // if the header pointed to by temp is free
        if (is_free(temp)) {
            // if there is enough space in the free block to service the request
            if (needed <= ((header *)temp)->size) {
                // calculate excess space in block considering that we do not want to leave space just for a header
                long space = ((header *)temp)->size - (needed + HEADER_SIZE);
                result = (char *)temp + HEADER_SIZE;  // create pointer to the place in the heap we will give to caller
                // if free block contains enough memory for the allocated block and a header
                if (space == 0) {
                    make_used(temp, needed + HEADER_SIZE);  // make the whole free block used
                    return result;
                    // if free block contains more memory than that needed for alocated block and a header
                } else if (space >= 0) {
                    make_used(temp, needed);  // allocate a block to the user
                    temp = (char *)temp + HEADER_SIZE + needed;  // update temp to point to rest of memory in free block
                    make_free(temp, space);  // make the rest of the block free
                    return result;
                    // if free block contains exactly enough memory for the allocated block
                } else {
                    make_used(temp, needed);  // make entire block used
                    return result;
                }
                // if free block does not have enough space for the user request
            } else {
                temp = (char *)temp + HEADER_SIZE + ((header *)temp)->size;  // point temp to next header
            }
            // if header indicates the block is used
        } else {
            temp = (char *)temp + HEADER_SIZE + (((header *)temp)->size - 1);  // point temp to next header
        }
    }
    // if result is not updated, the heap was exhausted, and we return NULL
    return result;
}

/* Function: myfree
-----------------------------
Given a pointer to the heap, myfree will free the memory pointed to by the pointer so that it can be allocated again.  myfree will do nothing if given a NULL pointer.
*/

void myfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    void *temp = (char *)ptr - HEADER_SIZE;  // create a pointer to the header of the inputted memory
    header *headerptr = (header *)temp;  
    (headerptr->size)--;  // make the header indicate free memory instead of used memory
}

/* Function: myrealloc
-----------------------------
Given a pointer to the heap, and a size, myrealloc will return a new pointer to the heap that contains size bytes with the memory from the old pointer copied to the new pointer.  myrealloc will then free the memory pointed to by the old pointer. If the inputted pointer is NULL, myrealloc will return a new pointer to heap with size bytes of memory.  If the inputted size is zero, myrealloc will free the memory pointed to by the inputted pointer.  If there is not enough space in heap to accomondate the requested memory, myrealloc will return NULL.
*/

void *myrealloc(void *old_ptr, size_t new_size) {
    if (old_ptr == NULL) {
        return mymalloc(new_size);
    }
    if (new_size == 0) {
        myfree(old_ptr);
        return NULL;
    }
    void *result = NULL;
    size_t needed = roundup(new_size, ALIGNMENT);  // align the inputed size with ALIGNMENT constant
    void *temp = segment_start;  // create a temporary pointer to traverse the heap
    void *end_heap =  (char *)segment_start + segment_size;  // create a pointer to the end of the heap
    // while there are headers to check
    while (temp < end_heap) {
        // if the header indicates free block of memory
        if (is_free(temp)) {
            // if the free block has enough memory to accomodate the realloc request
            if (needed <= ((header *)temp)->size) {
                // calculate excess space in block considering that we do not want to leave space just for a header
                long space = ((header *)temp)->size - (needed + HEADER_SIZE); 
                result = (char *)temp + HEADER_SIZE;  // update result to hold the place in heap we want to return
                void *old_header = (char *)old_ptr - 8;  // pointer to the header of the old block of memory
                size_t old_size = ((header *)old_header)->size;  // size of old block of memory
                size_t copy_size = 0;  // create a variable to hold how many bytes we want to copy to new block
                // if we are reallocating to smaller block 
                if (old_size >= new_size) {
                    copy_size = new_size;  // only new block amount of memory from old block
                    // if we are reallocating to bigger block
                } else {
                    copy_size = old_size;  // copy old block amount of memory to new block
                }
                memmove(result, old_ptr, copy_size);
                (((header *)old_header)->size)--;  // make old block free
                // if free block contains enough memory for the allocated block and a header
                if (space == 0) {
                    make_used(temp, needed + HEADER_SIZE);
                    return result;
                    // if there is more that the needed space in the free block
                } else if (space >= 0) {
                    make_used(temp, needed);
                    temp = (char *)temp + HEADER_SIZE + needed;
                    make_free(temp, space);  // free the remaining block in the free block by creating free header
                    return result;
                    // if there is exactly enough space in free block to accomadate just the requested size w/o header
                } else {
                    make_used(temp, needed);
                    return result;
                }
                // if free block does not have enough memory
            } else {
                temp = (char *)temp + HEADER_SIZE + ((header *)temp)->size;  // update temp to point to next header
            }
            // if block is used
        } else {
            temp = (char *)temp + HEADER_SIZE + (((header *)temp)->size - 1);  // update temp to point to next header
        }
    }
    // if the heap was exhausted, result will still equal NULL
    return result;
}

/* Function: validate_heap
-------------------------------
This function checks the internal structure of the heap by ensuring that all the memory of the heap is accounted for.  That is, all blocks are either allocated or freed.  If there is memory that is not accounted for, validate_heap returns false, and otherwise it returns true.
*/

bool validate_heap() {
    void *temp = segment_start;  // creating a temporary pointer to traverse the headers of the heap
    size_t count = 0;  // create a variable to keep track of the accountned for memory
    void *end_heap = (char *)segment_start + segment_size;
    // while there are still headers in the heap
    while (temp < end_heap) {
        header *cur_header = (header *)temp;
        size_t block_size = 0;  // create a variable to hold the size indicated by the header 
        if (is_free(temp)) {
            block_size = cur_header->size + HEADER_SIZE;  // add header size to block_size to account for headers
            count += block_size;
        } else {
            block_size = cur_header->size - 1 + HEADER_SIZE;  // subtract one because the header is used
            count += block_size;
        }
        temp = (char *)temp + block_size;  // move temp to next header
    }
    return (count == segment_size);  // checks if the memory used by the blocks equals the total memory
}

/* Function: dump_heap
 * -------------------
 * This function prints out the the block contents of the heap.  It is not
 * called anywhere, but is a useful helper function to call from gdb when
 * tracing through programs.  It prints out the total range of the heap, and
 * information about each block within it.
 */
void dump_heap() {
    void *temp = segment_start;  // creating a temporary pointer to traverse the heap
    void *end_heap = (char *)segment_start + segment_size;
    // while there are headers left in the heap
    while (temp < end_heap) {
        // for each header, we print the pointer to the header, whether the header is free or used, the decimal amount of bytes allocated by the header, and then the hex distance to the next header
        if (is_free(temp)) {
            printf("%p, %c, %ld, %zx\n", temp, 'f', ((header *)temp)->size, ((header *)temp)->size + HEADER_SIZE);
            temp = (char *)temp + HEADER_SIZE + ((header *)temp)->size;
        } else {
            printf("%p, %c, %ld, %zx\n", temp, 'u', (((header *)temp)->size) - 1, ((header *)temp)->size - 1 + HEADER_SIZE);
            temp = (char *)temp + HEADER_SIZE + (((header *)temp)->size - 1);
        }
    }        
}

/* CS 107
Code written by: Christo Hristov

This file contains a series of utility functions implemented to allocate, free, and reallocate memory from a heap.  These functions are used in the test_explicit.c file.
*/

#include <stdio.h>
#include <string.h>
#include "./allocator.h"
#include "./debug_break.h"

#define BLOCK_SIZE 8  // define a constant to hold the number of bytes in a block
#define MIN_BLOCK 24  // define a constant to hold the min number of bytes that can be allocated

static void *segment_start;
static size_t segment_size;
static void *first_free;

// create a struct, header, to hold the size of the block of memory indicated by the header
typedef struct {
    size_t size;  // number ending in one if the ehader is used and zero if the header is free
} header;

// create a struct to hold a node in the free linked list
typedef struct {
    void *next;  // pointer to next node in list
    void *prev;  // pointer to previous node in list
} node;

/* Function: roundup
----------------------------
Given an amount, sz, and a mulitplier, mult, roudnup returns the smallest number that is a multiple of mult and is larger or equal to sz.
*/

size_t roundup(size_t sz, size_t mult) {
    return (sz + mult - 1) & ~(mult - 1);
}

/* Given a void pointer, headerptr, is_free returns true if headerptr points to a header that indicates a free block, and false if headerptr points to a header that indicates a used block.

This function assumes that headerptr points to a header in the heap memory payload.
 */

bool is_free(void *headerptr) {
    header *header_ptr = (header *)headerptr;
    // checign if least significant digit is 0 or 1
    if ((header_ptr->size & 1) == 0) {
        return true;
    } else {
        return false;
    }
}

/* Function: make_free
----------------------------------
Given a pointer to where we want a free block, location, a size of the free block, space, a pointer to the next node in the free linked list, next_block, and a pointer to the previous node in the free linked list, prev_block, make_free will make a free block in the heap payload at location with the indicated size and update the free linked list to include this new block.

This function assumes that locatio is a memory address in the heap payload, that space is alligned, and that next_block and prev_block point to nodes in the free linked list or are NULL.
*/

void make_free(void *location, size_t space, void *next_block, void *prev_block) {
    header *new_header = (header *)location;  // make a block header for the free block
    new_header->size = space;  // make the block header contain the indicated size
    node *new_node = (node *)((char *)location + BLOCK_SIZE);  // create a new node for the free block
    // update the pointers of the node
    new_node->next = next_block;
    new_node->prev = prev_block;
    // if there is a previous block
    if (prev_block != NULL) {
        node *past_node = (node *)prev_block;  
        past_node->next = new_node;  // make previous block point to the new free block
        // if there is no previous block (the new block is the first free block)
    } else {
        first_free = location;  // update first_free global pointer
    }
    // if there is a next block in the linked list
    if (next_block != NULL) {
        node *next_node = (node *)next_block;
        next_node->prev = new_node;  // make previous pointer of next block point to new free block
    }
}

/* Function: coalesce
--------------------------------
Given a pointer to a free block, location,  coalesce will coalesce the inputted free block with any free blocks following the indicated block and touching.

This function assumes that location points to the header of a free block.
*/

void coalesce(void *location) {
    void *end_heap = (char *)segment_start + segment_size;
    header *cur_header = (header *)location;
    size_t cur_space = cur_header->size;
    size_t count = cur_space;  // create a variable count to keep track of total free space
    node *cur_node = (node *)((char *)location + BLOCK_SIZE);
    //  after we coalesce, the previous of the new block will equal the previous of the first coalesced block, and the next of the new block will equal the next of the last coalesced block
    void *prev_block = cur_node->prev;  // store previous of first coalesced block
    void *next_block = cur_node->next;  // store next of first coalesced block in case no blocks to be coalesced
    void *temp = (char *)location + count + BLOCK_SIZE;  // create a pointer to traverse heap
    // while consecutive free blocks are remianing
    while ((temp < end_heap) && is_free(temp)) {
        size_t new_space = ((header *)temp)->size + BLOCK_SIZE;
        count += new_space;  // update total space of coalesced blocks
        node *new_node = (node *)((char *)temp + BLOCK_SIZE);
        next_block = new_node->next;  // update next to point to next of last coalesced block
        temp = (char *)temp + new_space;  // update temp to point to next header
    }
    make_free(location, count, next_block, prev_block);  // create new coalesced free block
}

/* Function: remove_free
------------------------------
Given a pointer to a node of a free block, cur,  remove_free will remove that node from the free linked list.

This function assumes that cur is a pointer toa  node in the linked free list.
*/

void remove_free(node *cur) {
    node *next_block = (node *)(cur->next);  // find the next node
    node *prev_block = (node *)(cur->prev);  // find the previous node
    // if there is a previous node
    if (prev_block != NULL) {
        prev_block->next = next_block;  // make previous node point past the inputted node
        // if we are removing the first node
    } else {
        // if the node we are removing is the only node in the free list
        if (next_block == NULL) {
            first_free = (char *)segment_start + segment_size;  // update first_free to point to end of heap
            // there is another node that we are removing
        } else {
            first_free = (char *)next_block - BLOCK_SIZE;  // update first free to point after the node we remove
        }
    }
    //  if there is a node after the node we are removing
    if (next_block != NULL) {
        next_block->prev = prev_block;  // make the previous of the next node skip the node we are removing
    }
}

/* given a pointer, headerptr, and a requested size, allocated_size, make_used will change the header pointed to by headerptr to indicate a used block in memory with allocated_size bytes.

This function assumes that headerptr points to a header in the heap and that allocated_size is alligned and greater than MIN_BLOCK.
*/

void make_used(void *location, size_t allocated_size) {
    header *headerptr = (header *)location;
    headerptr->size = allocated_size + 1;  // add one to indicate a used block
}

/* Function: myinit
------------------------------
Given a pointer to the start of the heap, heap_start, and the size of the heap, heap_size, myinit intializes heap_size bytes of memory starting at heap_Start to be used as the heap.  Thsi is done by updateing global variables that refer to the size and start of the heap.  Subsequent calls to myinit will clear the ucrrent heap and reinialize a new heap with the inputted parameaters. myinit will return false if the inputted size is too small for the heap allcoator to use, and otherwise will return true.

This function assumes that heap_Start is a non null point that is alligned with the ALIGNMENT constant, and that heap_size is a multiple of ALIGNMENT.
*/

bool myinit(void *heap_start, size_t heap_size) {
    // if the heapsize is less than MIN_BLOCK, there is not enough memory to hold a header and a node
    if (heap_size <= 2 * BLOCK_SIZE) {
        return false;
    }
    segment_start = heap_start;
    segment_size = heap_size;
    first_free = segment_start;  // set first_free to point to begginging of heap as that is first free block
    header *first_header = (header *)segment_start;
    first_header->size = segment_size - BLOCK_SIZE;  // intialize header indicating that the whole block is free to use
    node *first_node = (node *)((char *)segment_start + BLOCK_SIZE);  // create first node in free linked list
    first_node->next = NULL;  // only free node so next and prev are NULL
    first_node->prev = NULL;
    return true;
}

/* Function: mymalloc
--------------------------
Given a number of bytes, requested_size, mymalloc will return a pointer to an adress in the heap that contains an alligned requested_size number of bytes to be used by the caller.  If the requested_size is 0 or there is not enough free memory in the heap to accomodate the user's request, mymalloc will return a null pointer.  If the user inputs a size less than MIN_BLOCK, mymalloc will allocate MIN_BLOCK number of bytes.
*/

void *mymalloc(size_t requested_size) {
    // if input is 0
    if (requested_size == 0) {
        return NULL;
    }
    void *result = NULL;
    // if input is less than MIN_BLOCK
    if (requested_size < MIN_BLOCK) {
        requested_size = MIN_BLOCK;
    }
    size_t needed = roundup(requested_size, ALIGNMENT);  // round how many bytes we need in memory
    node *temp = (node *)((char *)first_free + BLOCK_SIZE);  // create a temp variable to traverse the free linked list
    // while there are still free blocks
    while (temp != NULL) {
        header *free_header = (header *)((char *)temp - BLOCK_SIZE);
        size_t free_space = free_header->size;  // amount of space in the free block
        // if we have enough space in block to accomodate allocate request
        if (needed <= free_space) {
            long leftover_space = free_space - (needed + 2 * BLOCK_SIZE);  // we cannot leave 2 * BLOCK_SIZE bytes only
            result = temp;  // update result to point to memory where allocation will occur
            //  if there is enough space to allocate and we have to create free block
            if (leftover_space > 0) {
                // store  pointers of the current free header to use to update linked list with new created free block
                void *next_block = temp->next;  
                void *prev_block = temp->prev;
                make_used((char *)temp - BLOCK_SIZE, needed);  // make block used
                // create a free block with leftover space
                make_free((char *)temp + needed, (free_space - needed - BLOCK_SIZE), next_block, prev_block);
                // coalesce newly created free block
                coalesce((char *)temp + needed);
                return result;
                // do not have enough space to create a free block
            } else {
                remove_free(temp);  // remove free block and update linked list
                make_used((char *)temp - BLOCK_SIZE, free_space);  // make entire free block used
                return result;
            }
            // not enough space in free block for allocation request
        } else {
            temp = (node *)(temp->next);  // skip to next free block in linked list
        }
    }
    return result;
}

/* Given a pointer to the heap, ptr, myfree will free the memory pointed to by the pointer so that it an be allocated again.  myfree will do nothing if given NULL ptr.

This function assumes ptr points to the first address of a previously allocated block.
*/

void myfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    void *end_heap = (char *)segment_start + segment_size;  // create a pointer to end of heap
    ptr = (char *)ptr - BLOCK_SIZE;  // set ptr to used header
    void *free_location = ptr;  // set free_location to used header because that is what we will make_free
    size_t used_size = (((header *)free_location)->size) - 1;  // size of used block
    // search for next free block before end of list
    while (ptr < end_heap) {
        // if we find a free block
        if (is_free(ptr)) {
            // use this free block to get pointers to previous free block to add in new free block
            node *next_block = (node *)((char *)ptr + BLOCK_SIZE);
            void *prev_block = next_block->prev;
            make_free(free_location, used_size, next_block, prev_block);  // add in new free block
            // coalesce new free block
            coalesce(free_location);
            return;
            // if next header is not free
        } else {
            header *cur_header = (header *)ptr;
            size_t jump_space = cur_header->size + BLOCK_SIZE - 1;  
            ptr = (char *)ptr + jump_space;  // jump to next header
        }
    }
    //  if there are free blocks after the location we are freeing
    //  if there are no other free blocks in the heap
    if (first_free == end_heap) {
        make_free(free_location, used_size, NULL, NULL);  // make the only free block
        // by freeing we are creating the last free block
        // go through linked list to find current last free block to get pointers to update free linked list
    } else {
        node *temp = (node *)((char *)(first_free) + BLOCK_SIZE);  // first free node
        // while there are still blocks in free linked list
        while (temp->next != NULL) {
            temp = (node *)(temp->next);  // move to next free block
        }
        make_free(free_location, used_size, temp->next, temp);  // make free using pointers from last free block of list
    }
}

/* Function: myrealloc
-----------------------------
Given a pointer to the heap, old_ptr, and a size, new_size, myrealloc will change the old_ptr to point to the new_size amount of bytes and return old_ptr.  If there is not enough space at old_ptr for new_size amount of bytes, myrealloc will return a new pointer to a locaiton in the heap with new_size number of bytes and the memory from old_ptr copied.  myrealloc will then free the old memory used.  If there is not enough space in the heap for the request, myrealloc will not change the heap and return NULL.  If the inputted size is zero by realloc will free the meory pointed to by the inputted pointer.  If old_ptr is NULL, myrealloc will allocated new_size bytes of memory and will return the location of this memory on the heap.

This function assumes that old_ptr points to the beggining of a previously allocated block of memory.
*/

void *myrealloc(void *old_ptr, size_t new_size) {
    if (old_ptr == NULL) {
        return mymalloc(new_size);
    }
    if (new_size == 0) {
        myfree(old_ptr);
        return NULL;
    }
    // if the requested size is less than MIN_BLOCK
    if (new_size < MIN_BLOCK) {
        new_size = MIN_BLOCK;
    }
    size_t needed = roundup(new_size, ALIGNMENT);  // align the new_size
    header *old_header = (header *)((char *)old_ptr - BLOCK_SIZE);
    size_t free_space = old_header->size - 1;  // create a variable free space to keep track of space at old_ptr
    void *temp = (char *)old_ptr + free_space;  // create variable temp to traverse headers of heap
    void *result = NULL;
    // if the allocated memory is followed by a free block
    if (is_free(temp)) {
        coalesce(temp);
        header *free_header = (header *)temp;
        free_space += BLOCK_SIZE + free_header->size;  // update free space to account for following free blocks
        // if there is enough space for inplace realloc
        if (needed <= free_space) {
            // can't leave space for just 2 * BLOCK_SIZE bytes because this is less than MIN_BLOCK
            long leftover_space = free_space - (needed + 2 * BLOCK_SIZE);
            result = old_ptr;  // reallocating inplace so return same pointer
            node *free_node = (node *)((char *)temp + BLOCK_SIZE);
            // if space to create a free block after reallocation
            if (leftover_space > 0) {
                // use pointers from current free block to make new free block that fits in linked list
                void *next_block = free_node->next;
                void *prev_block = free_node->prev;
                make_used((char *)old_ptr - BLOCK_SIZE, needed);
                make_free((char *)old_ptr + needed, free_space - needed - BLOCK_SIZE, next_block, prev_block);
                return result;
                // not enough space for new free block after reallocation
            } else {
                remove_free(free_node);  // remove entire free block from linked list
                make_used(old_header, free_space);  // make entire free block used
                return result;
            }
            // if there is not enough space for inplace realloc
        } else {
            result = mymalloc(needed);  // allocate memory somewhere else
            // if heap is exhasuted
            if (result == NULL) {
                return old_ptr;
                // if space on heap for reallocation
            } else {
                memmove(result, old_ptr, old_header->size);  // copy memory to new location
                myfree(old_ptr);  // free the old location
                coalesce((char *)old_ptr - BLOCK_SIZE);  // coalesece newly freed block 
                return result;  // return new location
            }
        }
        // if used block is followed by another used block
    } else {
        // if we have space for inplace realloc
        if (needed <= free_space) {
            // we can't just leave space for < MIN_BLOCK bytes
            long leftover_space = free_space - (needed + 2 * BLOCK_SIZE);
            // have space for free block after reallocaiton
            if (leftover_space > 0) {
                make_used((char *)old_ptr - BLOCK_SIZE, needed);  // reallocate memory
                // make the reamining memory used so that we can free it
                make_used((char *)old_ptr + needed, free_space - needed - BLOCK_SIZE);
                // free reaminign memory so that it is correctly included in free linked lsit
                myfree((char *)old_ptr + needed + BLOCK_SIZE);
                return old_ptr;
                // if we dont need to create a free block
            } else {
                return old_ptr;  // we still need to reallocate the same amount as previously allocated
            }
            // if not enough space for inplace realloc
        } else {
            result = mymalloc(needed);  // allocate space somewhere else on heap
            // if heap exhausted
            if (result == NULL) {
                return old_ptr;
            } else {
                memmove(result, old_ptr, old_header->size);  // copy memory to new location
                myfree(old_ptr);  // free old allcoated space
                return result;
            }
        }
    }
}
                
/* Function: validate_heap
---------------------------------
This function checks the internal structure of the heap by ensuring that all the memory of the heap is accounted for and that the free linked list contains all the free blocks in the correct order.  If there is memory that is not accounted for or a free block that is not on the list, or a node on the list that does not correspond to the correct free block, validate_heap returns false, otherwise if returns true.
*/

bool validate_heap() {
    void *temp = segment_start;  // create a pointer to traverse headers of list
    size_t count = 0;  // create a variable to count accounted for bytes
    void *end_heap = (char *)segment_start + segment_size;
    node *cur_node = (node *)((char *)first_free + BLOCK_SIZE);  // create a pointer to traverse free linked list
    // while there are headers to be read
    while (temp < end_heap) {
        header *cur_header = (header *)temp;
        size_t block_size = 0;
        // if the header is free
        if (is_free(temp)) {
            block_size = cur_header->size + BLOCK_SIZE;
            count += block_size;  // update the amount to account for free bytes
            // if the current free block does not correspond to the current node in the free list
            if (temp != ((char *)cur_node - BLOCK_SIZE)) {
                return false;
            } else {
                cur_node = (node *)(cur_node->next);  // update node to point to next node in linked list
            }
        } else {
            block_size = cur_header->size - 1 + BLOCK_SIZE;
            count += block_size;  // update count to account for allocated bytes
        }
        temp = (char *)temp + block_size;  // point temp to next header
    }
    // if there are no more free blocks but more free nodes in the linked list
    if (cur_node != NULL) {
        return false;
    }
    return (count == segment_size);  //  checks if memory used by the blocks equals the total memory
}

/* Function: dump_heap
 * -------------------
 * This function prints out the the block contents of the heap.  For all headers, this function prints out the pointer to the header, a character indicating that it is free or used, the size of the block, and the amount of bytes in hex until the next header.  If the header is free, dump_heap also prints out the current node, the next node, and the previous node in the free linked list.  dump_heap is not
 * called anywhere, but is a useful helper function to call from gdb when
 * tracing through programs.  It prints out the total range of the heap, and
 * information about each block within it.
 */
void dump_heap() {
    void *temp = segment_start;
    void *end_heap = (char *)segment_start + segment_size;
    printf("%s: %p\n", "pointer to first free header", first_free);  // print out pointer to first free block
    // while there are headers in the heap
    while (temp < end_heap) {
        if (is_free(temp)) {
            printf("%p, %c, %ld, %zx, ", temp, 'f', ((header *)temp)->size, ((header *)temp)->size + BLOCK_SIZE);
            node *cur_node = (node *)((char *)temp + BLOCK_SIZE);
            printf("%p, %p, %p\n", cur_node, cur_node->next, cur_node->prev);
            temp = (char *)temp + BLOCK_SIZE + ((header *)temp)->size;
        } else {
            size_t block_len = (((header *)temp)->size) - 1;
            printf("%p, %c, %ld, %zx\n", temp, 'u', block_len, block_len + BLOCK_SIZE);
            temp = (char *)temp + BLOCK_SIZE + block_len;
        }
    }
}

#include "./allocator.h"
#include "./debug_break.h"
#include <stdio.h>
#include <string.h>

#define HEADER_SIZE 8
static void *segment_start;
static size_t segment_size;

typedef struct {
    size_t size;
} header;

size_t roundup(size_t sz, size_t mult) {
    return (sz + mult - 1) & ~(mult - 1);
}

bool is_free(void *headerptr) {
    header *header_ptr = (header *)headerptr;
    if ((header_ptr->size & 1) == 0) {
        return true;
    } else {
        return false;
    }
}

void make_used(void *headerptr, size_t requested_size) {
    header *header_ptr = (header *)headerptr;
    header_ptr->size = requested_size + 1;
}

void make_free(void *ptr, size_t space) {
    header *header_ptr = (header *)ptr;
    header_ptr->size = space;
}
    
bool myinit(void *heap_start, size_t heap_size) {
    /* TODO(you!): remove the line below and implement this!
     * This must be called by a client before making any allocation
     * requests.  The function returns true if initialization was
     * successful, or false otherwise. The myinit function can be
     * called to reset the heap to an empty state. When running
     * against a set of of test scripts, our test harness calls
     * myinit before starting each new script.
     */
    if (heap_size <= HEADER_SIZE) {
        return false;
    }
    segment_start = heap_start;
    segment_size = heap_size;
    header *first_header = (header *)heap_start;
    first_header->size = heap_size - HEADER_SIZE;
    return true;
}

void *mymalloc(size_t requested_size) {
    void *result = NULL;
    if (requested_size == 0) {
        return result;
    }
    size_t needed = roundup(requested_size, ALIGNMENT);
    void *temp = segment_start;
    void *end_heap =  (char *)segment_start + segment_size;
    while (temp < end_heap) {
        if (is_free(temp)) {
            if (needed <= ((header *)temp)->size) {
                long space = ((header *)temp)->size - (needed + HEADER_SIZE);
                result = (char *)temp + HEADER_SIZE;
                if (space == 0) {
                    make_used(temp, needed + HEADER_SIZE);
                    return result;
                } else if (space >= 0) {
                    make_used(temp, needed);
                    temp = (char *)temp + HEADER_SIZE + needed;
                    make_free(temp, space);
                    return result;
                } else {
                    make_used(temp, needed);
                    return result;
                }
            } else {
                temp = (char *)temp + HEADER_SIZE + ((header *)temp)->size;
            }
        } else {
            temp = (char *)temp + HEADER_SIZE + (((header *)temp)->size - 1);
        }
    }
    return result;
}
                
void myfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    void *temp = (char *)ptr - HEADER_SIZE;
    header *headerptr = (header *)temp;
    (headerptr->size)--;
}

void *myrealloc(void *old_ptr, size_t new_size) {
    void *result = NULL;
    size_t needed = roundup(new_size, ALIGNMENT);
    void *temp = segment_start;
    void *end_heap =  (char *)segment_start + segment_size;
    while (temp < end_heap) {
        if (is_free(temp)) {
            if (needed <= ((header *)temp)->size) {
                long space = ((header *)temp)->size - (needed + HEADER_SIZE);
                result = (char *)temp + HEADER_SIZE;
                void *old_header = (char *)old_ptr - 8;
                size_t old_size = ((header *)old_header)->size;
                memmove(result, old_ptr, old_size);
                (((header *)old_header)->size)--;
                if (space == 0) {
                    make_used(temp, needed + HEADER_SIZE);
                    return result;
                } else if (space >= 0) {
                    make_used(temp, needed);
                    temp = (char *)temp + HEADER_SIZE + needed;
                    make_free(temp, space);
                    return result;
                } else {
                    make_used(temp, needed);
                    return result;
                }
            } else {
                temp = (char *)temp + HEADER_SIZE + ((header *)temp)->size;
            }
        } else {
            temp = (char *)temp + HEADER_SIZE + (((header *)temp)->size - 1);
        }
    }
    return result;
}

bool validate_heap() {
    /* TODO(you!): remove the line below and implement this to
     * check your internal structures!
     * Return true if all is ok, or false otherwise.
     * This function is called periodically by the test
     * harness to check the state of the heap allocator.
     * You can also use the breakpoint() function to stop
     * in the debugger - e.g. if (something_is_wrong) breakpoint();
     */
    
    return true;
}

/* Function: dump_heap
 * -------------------
 * This function prints out the the block contents of the heap.  It is not
 * called anywhere, but is a useful helper function to call from gdb when
 * tracing through programs.  It prints out the total range of the heap, and
 * information about each block within it.
 */
void dump_heap() {
    void *temp = segment_start;
    void *end_heap = (char *)segment_start + segment_size;
    while (temp < end_heap) {
        if (is_free(temp)) {
            printf("%p, %c, %xld\n", temp, 'f', ((header *)temp)->size);
            temp = (char *)temp + HEADER_SIZE + ((header *)temp)->size;
        } else {
            printf("%p, %c, %xld\n", temp, 'u', (((header *)temp)->size) - 1);
            temp = (char *)temp + HEADER_SIZE + (((header *)temp)->size - 1);
        }
    }        
}

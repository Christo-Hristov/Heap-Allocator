#include "./allocator.h"
#include "./debug_break.h"

#define BLOCK_SIZE 8

static void *segment_start;
static size_t segment_size;
static void *first_free;

typedef struct {
    size_t size;
} header;

typedef struct {
    void *next;
    void *prev;
} node;

size_t roundup(size_t sz, size_t mult) {
    return (sz + mult - 1) & ~(mult - 1);
}

void make_free(void *location, size_t space, void *next_block, void *prev_block) {
    header *new_header = (header *)location;
    new_header->size = space;
    node *new_node = (node *)((char *)location + BLOCK_SIZE);
    new_node->next = next_block;
    new_node->prev = prev_block;
    if (prev_block != NULL) {
        node *past_node = (node *)prev_block;
        past_node->next = new_node;
        //node past_node = *prev_block;
        //past_node.next = new_node;
    } else {
        first_free = location;
    }
    if (next_block != NULL) {
        node *next_node = (node *)next_block;
        next_node->prev = new_node;
        //node next_node = *next_block;
        //next_node.prev = new_node;
    }
    //coalesce
}

void remove_free(node *cur) {
    node *next_block = (node *)(cur->next);
    node *prev_block = (node *)(cur->prev);
    if (prev_block != NULL) {
        prev_block->next = next_block;
    } else {
        first_free = (char *)next_block - BLOCK_SIZE;
    }
    if (next_block != NULL) {
        next_block->prev = prev_block;
    }
}
    
void make_used(void *location, size_t allocated_size) {
    header *headerptr = (header *)location;
    headerptr->size = allocated_size + 1;
}
       
bool myinit(void *heap_start, size_t heap_size) {
    if (heap_size <= 2 * BLOCK_SIZE) {
        return false;
    }
    segment_start = heap_start;
    segment_size = heap_size;
    first_free = segment_start;
    header *first_header = (header *)segment_start;
    first_header->size = segment_size - BLOCK_SIZE;
    node *first_node = (node *)((char *)segment_start + BLOCK_SIZE);
    first_node->next = NULL;
    first_node->prev = NULL;
    return true;
}

void *mymalloc(size_t requested_size) {
    void *result = NULL;
    size_t needed = roundup(requested_size, ALIGNMENT);
    node *temp = (node *)((char *)first_free + BLOCK_SIZE);
    while (temp != NULL) {
        header *free_header = (header *)((char *)temp - BLOCK_SIZE);
        size_t free_space = free_header->size;
        if (needed <= free_space) {
            long leftover_space = free_space - (needed + 2 * BLOCK_SIZE);
            result = temp;
            if (leftover_space > 0) {
                void *next_block = temp->next;
                void *prev_block = temp->prev;
                make_used((char *)temp - BLOCK_SIZE, needed);
                make_free((char *)temp + needed, (free_space - needed - BLOCK_SIZE), next_block, prev_block);
                return result;
            } else {
                remove_free(temp);
                make_used((char *)temp - BLOCK_SIZE, free_space);
                return result;
            }
        } else {
            temp = (node *)(temp->next);
        }
    }
    return result;
}

void myfree(void *ptr) {
    // TODO(you!): implement this!
}

void *myrealloc(void *old_ptr, size_t new_size) {
    // TODO(you!): remove the line below and implement this!
    return NULL;
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
    return false;
}

/* Function: dump_heap
 * -------------------
 * This function prints out the the block contents of the heap.  It is not
 * called anywhere, but is a useful helper function to call from gdb when
 * tracing through programs.  It prints out the total range of the heap, and
 * information about each block within it.
 */
void dump_heap() {
    // TODO(you!): Write this function to help debug your heap.
}

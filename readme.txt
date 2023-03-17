File: readme.txt
Author: Christo Hristov
----------------------

1. I created a struct called node to represent a node in the doubly linked free list.  This struct consisted of a pointer to the next node and a pointer to the previous node.  This allowed me to traverse the list without performing extensive pointer arithmetic.  I additionally ommitted variables pertaining to used/free bytes of the heap and only used pointers to create the heap allocator.  This decreased the complexity of the code as I did not have to worry about updating these variables in every function call.  My allocator would show strong performance on the following script: a 1 300, because this call would satisfy the first condition of malloc so very few instructions would have to be executed.  My allocator would show weak performance on the following script:
a 1 300
a 2 segment_size - 300
f 1
f 2
This is because myfree for the second pointer would start at the begginging of the linked free list and traverse through the entire list to find the last free node.  It would then have to use the information of this node to free the last allocated segment.  I call coalesce in realloc before determining if there is enough space for an inplace realloc, so that I don't have to manually count if there is enough space for an inplace realloc and then manually count the following free blocks again when I coalesce.


2. My allocator assumes the the old_ptr passed into myfree points directly after the header for that used block.  If the user passes in a different pointer that points to an arbitrary location in the heap, myfree will attempt to cast random memory to headers and random memory to nodes which will corrupt the heap and cause a segmentation fault.  My allocator also assumes that the old_ptr passed into realloc points directly after the header for that allocated block.  So again, if the user passes in an arbitry pointer, myrealloc will cast and derefece random memory which will lead to a segmentation fault.


Tell us about your quarter in CS107!
-----------------------------------
I am proud of this last assignment because you were given the flexibility to build the heap allocator how you wanted.  This meant simple things such as creating structs to represent values was more satisfying because you came up with it yourself.  The course taught me a lot and was well organized.  It did seem that at times some aspects were busy work such as adding so many comments after already spending so much time writing code.



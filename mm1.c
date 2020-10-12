/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "team name",
    /* First member's full name */
    "member 1",
    /* First member's email address */
    "member_1@cse.iitb.ac.in",
    /* Second member's full name (leave blank if none) */
    "member 2",
    /* Second member's email address (leave blank if none) */
    "member_2@cse.iitb.ac.in"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WPTR unsigned int *
#define WORD_SIZE 4 // 1W = 4B
#define FTR_SIZE 4
#define HDR_SIZE 4

// Get and set word pointed by ptr
#define GET_WORD(ptr) (*(unsigned int *)(ptr))
#define SET_WORD(ptr, data) ((*(unsigned int *)(ptr)) = data)

// Get block size and alloc bit
#define GET_BLOCK_SIZE(ptr) (GET_WORD(ptr) & ~0x1)
#define GET_ALLOC(ptr) (GET_WORD(ptr) & 0x1)

// Get and set block header and footer
#define GET_HDR(ptr) (*(unsigned int *)(ptr))
#define SET_HDR(ptr, data) (*(unsigned int *)(ptr) = data)
#define GET_FTR(ptr) (*(unsigned int *)((char*)(ptr) + GET_BLOCK_SIZE(ptr) - FTR_SIZE))
#define SET_FTR(ptr, data) (*(unsigned int *)((char*)(ptr) + GET_BLOCK_SIZE(ptr) - FTR_SIZE) = data)

// Get and set Next/Prev pointers in free blocks
#define GET_NEXT(ptr) (*((unsigned int *)(ptr) + 1))
#define GET_PREV(ptr) (*((unsigned int *)(ptr) + 2))
#define SET_NEXT(ptr, data) (*((unsigned int *) ptr + 1) = data)
#define SET_PREV(ptr, data) (*((unsigned int *) ptr + 2) = data)

// Get actual next & prev blocks
#define GET_ANEXT(ptr) (GET_BLOCK_SIZE(ptr) + (char*)(ptr)) // Convert to 1B pointer, move ahead BLOCK_SIZE bytes
#define GET_APREV(ptr) ((char*)(ptr) - GET_BLOCK_SIZE((char*)(ptr) - FTR_SIZE)) // Convert to 1B pointer, move back 4 bytes, get block size and move back BLOCK_SIZE bytes

// Format header
#define FHDR(size, a) (size | a)

// Min no bytes to extend heap
#define EXTEND_BY_SIZE 1 << 12 // in bytes

// Utility to find maximum
#define max(x, y) x > y ? x : y

// Minimum bytes for a free block
#define MIN_FREE_BLOCK_SIZE 16 // 16 = 4W = HDR + FTR + NEXT + PREV


void add_block_to_fl(void*);
void remove_block_from_fl(void*);
void* move_pbrk(int);
void* coalesce(void*);

void* fl_head;


/*
	Adds a free block to start of the free list.
*/
void add_block_to_fl(void* block) {
	if (!block) { // Safety check
		return;
	}

	if (!fl_head) {
		fl_head = block;
		SET_PREV(fl_head,NULL);
		SET_NEXT(fl_head,NULL);
		return;
	}

	SET_PREV(fl_head, block);
	SET_NEXT(block, fl_head);
	
	SET_PREV(block,NULL);

	fl_head = block;
}

/*
	Uses the mem_sbrk(bytes) call to move the program break. 
	Treats the extended chunk as a new free block.
	Coalesces with the previous free block (if any).
	Returns the updated free block if coalescing is done.
*/
void* move_pbrk(int bytes) {
	// Align to nearest multiple of 'alignment'
	bytes = ALIGN(bytes);

	// Request for space
	void* bptr = mem_sbrk(bytes);
	if (((int)(bptr)) == -1) { // Memory overflow
		return NULL;
	}

	// Initialise new free block
	SET_HDR(bptr, FHDR(bytes, 0));
	SET_FTR(bptr, FHDR(bytes, 0));
	SET_NEXT(bptr, 0);
	SET_PREV(bptr, 0);

	// Coalesce with prev free block (if any)
	bptr = coalesce(bptr);

	// Add block to list
	add_block_to_fl(bptr);

	return bptr;
}


/*
	Removes a block from the free list.
*/
void remove_block_from_fl(void *block) {
	// TODO: Safety check if required

	// Case 1: Only node i.e next = 0 & prev = 0
	if (!GET_NEXT(block) && !GET_PREV(block)) {
		fl_head = NULL;
		return;
	}

	// Case 2: Head node i.e. prev = 0 and next != 0
	if (!GET_PREV(block) && GET_NEXT(block)) {
		fl_head = GET_NEXT(block);
		SET_PREV(fl_head, NULL);
		return;
	}

	// Case 3: Last node i.e prev != 0 and next = 0
	if (GET_PREV(block) && !GET_NEXT(block)) {
		SET_NEXT(GET_PREV(block), 0);
		return;
	}
	
	// Case 4: Middle node
	void* prev_block = GET_PREV(block);
	void* next_block = GET_NEXT(block);

	SET_NEXT(prev_block, next_block);
	SET_PREV(next_block, prev_block);
}

/*
	Takes as input a free block and tries to coalesce with adjacent free blocks if any.
	Returns updated block if any coalescing was done.
*/
void* coalesce(void* bptr) {
	void* next_block = GET_ANEXT(bptr);
	void* prev_block = GET_APREV(bptr);
	
	int next_a = next_block ? GET_ALLOC(next_block) : 0;
	int prev_a = prev_block ? GET_ALLOC(prev_block) : 0;
	if (next_block == mem_sbrk(0))		//this is the last block
		next_a = 1;

	unsigned int next_size = next_block ? GET_BLOCK_SIZE(next_block) : 0;
	unsigned int prev_size = prev_block ? GET_BLOCK_SIZE(prev_block) : 0;

	int cur_size = GET_BLOCK_SIZE(bptr);

	// Case 1: Prev and next both are allocated, do nothing
	if (!next_a && prev_a) { // Case 2: Prev is allocated, next is free
		remove_block_from_fl(next_block);
		cur_size += next_size;
		// Order is important sinze, FTR location calculation makes use of size in HDR
		SET_HDR(bptr, FHDR(cur_size, 0));
		SET_FTR(bptr, FHDR(cur_size, 0));

	} else if (next_a && !prev_a) { // Case 3: Next is allocated, prev is free
		remove_block_from_fl(prev_block);
		cur_size += prev_size;
		// Order is important sinze, FTR location calculation makes use of size in HDR
		SET_HDR(prev_block, FHDR(cur_size, 0));
		SET_FTR(prev_block, FHDR(cur_size, 0));
		
		bptr = prev_block;

	} else if (!next_a && !prev_a){ // Case 4: Prev and next both are free
		remove_block_from_fl(next_block);
		remove_block_from_fl(prev_block);
		cur_size += (next_size + prev_size);
		// Order is important since, FTR location calculation makes use of size in HDR
		SET_HDR(prev_block, FHDR(cur_size, 0));
		SET_FTR(prev_block, FHDR(cur_size, 0));

		bptr = prev_block;
	}

	return bptr;
}

/*
	Finds the best block to allocate
	bytes = raw number of bytes required (i.e. including header and footer)
*/
void* best_fit(int bytes) {
	void* itr = fl_head;
	if (!fl_head) { // Safety check
		return NULL;
	}
	
	unsigned int best_size = 1 << 31;
	void* ret = NULL;

	while (itr != 0) {
		int cur_size = GET_BLOCK_SIZE(itr);
		if (cur_size >= bytes && cur_size < best_size) {
			best_size = cur_size;
			ret = itr;
		}

		itr = GET_NEXT(itr);
	}

	return ret;
}

/*
	Allocate and split block if required
	bytes = raw number of bytes required (i.e. including header and footer)
*/
void allocate(void* ptr, int bytes) {
	int block_size = GET_BLOCK_SIZE(ptr);
	remove_block_from_fl(ptr);

	// Case 1: BLOCK SIZE = bytes, no splitting required 
	if (block_size - bytes <= MIN_FREE_BLOCK_SIZE) {
		SET_HDR(ptr, FHDR(block_size, 1));
		SET_FTR(ptr, FHDR(block_size, 1));

	} else { // Case 2: BLOCK_SIZE > bytes, split, remove unused chunk and add to the beginning of the list
		SET_HDR(ptr, FHDR(bytes, 1));
		SET_FTR(ptr, FHDR(bytes, 1));

		void* next_bptr = GET_ANEXT(ptr);
		SET_HDR(next_bptr, FHDR(block_size - bytes, 0));
		SET_FTR(next_bptr, FHDR(block_size - bytes, 0));
		SET_NEXT(next_bptr, 0);
		SET_PREV(next_bptr, 0);

		next_bptr = coalesce(next_bptr);

		add_block_to_fl(next_bptr);
	}
	
}





/* 
 * mm_init - initialize the malloc package.
 */

void *init_mem_sbrk_break = NULL;


int mm_init(void)
{
	
	//This function is called every time before each test run of the trace.
	//It should reset the entire state of your malloc or the consecutive trace runs will give wrong answer.	
	

	/* 
	 * This function should initialize and reset any data structures used to represent the starting state(empty heap)
	 * 
	 * This function will be called multiple time in the driver code "mdriver.c"
	 */

	mem_reset_brk();
	void *ptr = (char*)mem_sbrk(0);
	int mask = (int)((unsigned int)ptr&0x7);
	if (mask == 0)
		ptr = mem_sbrk(4);
	else if (mask >= 1 && mask <= 4)
		ptr = mem_sbrk(4 - mask); 
	else
		ptr = mem_sbrk(12 - mask);

	/* Make the program break divisible by 4 but not by 8 (i.e. ALIGNMENT). 
	 * Since we always allocate multiple of 8B blocks (including header & footer), the same constraint will be maintained afterwards.
	 * Reason: When we allocate blocks we need to allocate 4B of block header as well.
	 * So everytime the pointer to the actual block (not block header) is point to address which is multiple of 8 (that means aligned by 8 bytes)
	 */
	fl_head = NULL;

	// Dummy block to prevent segment overflow error
	unsigned int sz = 8;
	void* dbptr = mem_sbrk(sz);
	SET_HDR(dbptr, FHDR(sz, 1));
	SET_FTR(dbptr, FHDR(sz, 1));


	void* res = move_pbrk(EXTEND_BY_SIZE);

	if (res == NULL) {
		printf("Memory full");
		return -1;
	}
	
    return 0;		//Returns 0 on successfull initialization.
}

//---------------------------------------------------------------------------------------------------------------
/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{	
	/* 
	 * This function should keep track of the allocated memory blocks.
	 * The block allocation should minimize the number of holes (chucks of unusable memory) in the heap memory.
	 * The previously freed memory blocks should be reused.
	 * If no appropriate free block is available then the increase the heap  size using 'mem_sbrk(size)'.
	 * Try to keep the heap size as small as possible.
	 */

	if(size <= 0){		// Invalid request size
		return NULL;
	}
	size = ((size+7)/8)*8;		//size alligned to 8 bytes

	unsigned int req_size = size + 8; // Adjusted to accomodate header and footer

	// Search for block in free list
	void* best_block = best_fit(req_size);
	
	if (best_block != NULL) {
		allocate(best_block, req_size);
		return (void*)((char*)best_block + 4); 
	}
	
	// Try to extend program break
	best_block = move_pbrk(max(req_size, EXTEND_BY_SIZE));
	if (best_block != NULL) {
		allocate(best_block, req_size);
		return (void*)((char*)best_block + 4); 
	}

	printf("Could not allocate block\n");
	return NULL;
	
	// return mem_sbrk(size);		//mem_sbrk() is wrapper function for the sbrk() system call. 
								//Please use mem_sbrk() instead of sbrk() otherwise the evaluation results 
								//may give wrong results
}


void mm_free(void *ptr)
{
	/* 
	 * Searches the previously allocated node for memory block with base address ptr.
	 * 
	 * It should also perform coalesceing on both ends i.e. if the consecutive memory blocks are 
	 * free(not allocated) then they should be combined into a single block.
	 * 
	 * It should also keep track of all the free memory blocks.
	 * If the freed block is at the end of the heap then you can also decrease the heap size 
	 * using 'mem_sbrk(-size)'.
	 */

	if (ptr == NULL) { 
		return;
	}

	// Possibly add a check to make sure that only an allocated block is being freed

	// Set allocated bit to zero and coalesce
	void* bptr = (void*)((char*) ptr - 4); // move back 4 bytes
	SET_HDR(bptr, FHDR(GET_BLOCK_SIZE(bptr), 0));
	SET_FTR(bptr, FHDR(GET_BLOCK_SIZE(bptr), 0));
	SET_NEXT(bptr, 0);
	SET_PREV(bptr, 0);

	bptr = coalesce(bptr);

	add_block_to_fl(bptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{	
	size = ((size+7)/8)*8; //8-byte alignement	
	unsigned int req_size = size + 8; // Adjusted to accomodate header and footer
	
	void* bptr = (void*)((char*) ptr - 4); // move back 4 bytes

	if(ptr == NULL) {	//memory was not previously allocated
		return mm_malloc(size);
	}
	
	if(size == 0){				//new size is zero
		mm_free(ptr);
		return NULL;
	}

	/*
	 * This function should also copy the content of the previous memory block into the new block.
	 * You can use 'memcpy()' for this purpose.
	 * 
	 * The data structures corresponding to free memory blocks and allocated memory 
	 * blocks should also be updated.
	*/

	int block_size = GET_BLOCK_SIZE(bptr);
	if (req_size <= block_size) {
	        // Case 1: no splitting required 
		if (block_size - size <= MIN_FREE_BLOCK_SIZE) {
                	SET_HDR(bptr, FHDR(block_size, 1));
                	SET_FTR(bptr, FHDR(block_size, 1));
		}
        	else { // Case 2: BLOCK_SIZE > size, split, remove unused chunk and add to the beginning of the list
                	SET_HDR(bptr, FHDR(req_size, 1));
                	SET_FTR(bptr, FHDR(req_size, 1));
                
			void* next_bptr = GET_ANEXT(bptr);
                	SET_HDR(next_bptr, FHDR(block_size - req_size, 0));
                	SET_FTR(next_bptr, FHDR(block_size - req_size, 0));
                	SET_NEXT(next_bptr, 0);
                	SET_PREV(next_bptr, 0);

                	next_bptr = coalesce(next_bptr);

                	add_block_to_fl(next_bptr);
        	}
		return ptr;
	}
	else { // Allocate more memory
		void *pbrk = mem_sbrk(0);
                if (pbrk == GET_ANEXT(bptr)) // Extend the block
                {
                        void *newptr = move_pbrk(max(req_size - block_size, EXTEND_BY_SIZE));
                        if (newptr != NULL) {
                                allocate(newptr, req_size);
                                return (void*)((char*)newptr + 4);
                        }

                        return NULL;

                }
                else { // allocate a new block
                        void *newptr = mm_malloc(size);         // mm_malloc() will take care of the header & footer part
                        if ( newptr == NULL)
                                return NULL;
                        memcpy(newptr, ptr, block_size - 8); // we don't have to copy header & footer
                        mm_free(ptr);
			return newptr;
		}
	}
}















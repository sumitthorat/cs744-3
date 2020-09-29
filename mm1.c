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

#define WPTR (unsigned int *)
#define WORD_SIZE 4 // 1W = 4B

// Get and set word pointed by ptr
#define GET_WORD(ptr) (*(unsigned int *) ptr)
#define SET_WORD(ptr, data) ((*(unsigned int *)(ptr)) = data)

// Get block size and alloc bit
#define GET_BLOCK_SIZE(ptr) (GET_WORD(ptr) & ~0x1)
#define GET_ALLOC(ptr) (GET_WORD(ptr) & 0x1)

// Get and set block header and footer
#define GET_HDR(ptr) (*(unsigned int *)(ptr))
#define SET_HDR(ptr, data) (*(unsigned int *)(ptr) = data)
#define GET_FTR(ptr) (*(unsigned int *)((char*)(ptr) + GET_BLOCK_SIZE(ptr) - 4))
#define SET_FTR(ptr, data) (*(unsigned int *)((char*)(ptr) + GET_BLOCK_SIZE(ptr) - 4) = data)

// Get and set Next/Prev pointers in free blocks
#define GET_NEXT(ptr) (*(unsigned int *)(ptr) + 1)
#define GET_PREV(ptr) (*(unsigned int *)(ptr) + 2)
#define SET_NEXT(ptr, data) (*((unsigned int *) ptr + 1) = data)
#define SET_PREV(ptr, data) (*((unsigned int *) ptr + 2) = data)

// Get actual next & prev blocks
#define GET_ANEXT(ptr) (GET_BLOCK_SIZE(ptr) + (char*)(ptr)) // Convert to 1B pointer, move ahead BLOCK_SIZE bytes
#define GET_APREV(ptr) ((char*)(ptr) - GET_BLOCK_SIZE((char*)(ptr) - 4)) // Convert to 1B pointer, move back 4 bytes, get block size and move back BLOCK_SIZE bytes

// Format header
#define FHDR(size, a) (size | a)


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
	
	return mem_sbrk(size);		//mem_sbrk() is wrapper function for the sbrk() system call. 
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
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{	
	size = ((size+7)/8)*8; //8-byte alignement	
	
	if(ptr == NULL){			//memory was not previously allocated
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

	mm_free(ptr);
	return mem_sbrk(size);
	
}















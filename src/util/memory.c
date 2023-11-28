#include "memory.h"
#include <ultratypes.h>

struct HeapSegment *gFirstFreeSegment;
void *gHeapStart;
void *gHeapEnd;

/**
 * @brief Initializes a heap memory segment.
 * 
 * 
 * @param segment HeapSegment struct pointer
 * @param end pointer to the end of the segment
 * @param type indication of the type of segment (used or free)
 */
void heapInitBlock(struct HeapSegment *segment, void *end, int type)
{
	//add header to the start of the segment that indicates the type and the end of the segment.
	segment->header = type | MALLOC_BLOCK_HEAD;
	segment->segmentEnd = end;

	//if the segment is free set the next and previous segments to 0 since they are not linked yet.
	if (type == MALLOC_FREE_BLOCK)
	{
		segment->nextSegment = 0;
		segment->prevSegment = 0;
	}
	//add footer to the end of the segment that indicates the type and points to the header of the segment.
	struct HeapSegmentFooter *footer = (struct HeapSegmentFooter *)segment->segmentEnd - 1;
	footer->footer = type | MALLOC_BLOCK_FOOT;
	footer->header = segment;
}

/**
 * @brief initializes the heap given a start end end pointer.
 * 
 * @param heapStart 
 * @param heapEnd 
 */
void heapInit(void *heapStart, void *heapEnd)
{
	//align the start of the heap to 8 bytes by adding 7 to the start address and masking the lower 3 bits.
	gFirstFreeSegment = (struct HeapSegment *)(((int)heapStart + 7) & ~0x7);
	//create a free block the size of the heap
	heapInitBlock(gFirstFreeSegment, heapEnd, MALLOC_FREE_BLOCK);
	//update the heap start and end pointers
	gHeapStart = gFirstFreeSegment;
	gHeapEnd = heapEnd;
}

/**
 * @brief Will reset the entire heap.
 * 
 */
void heapReset()
{
	heapInit(gHeapStart, gHeapEnd);
}

/**
 * @brief creates a cache free pointer for the target.
 * 
 * @param target 
 * @return void* 
 */
void *cacheFreePointer(void *target)
{
	return (void *)(((int)target & 0x0FFFFFFF) | 0xA0000000);
}

/**
 * @brief Removes a segment from the linked list of free segments.
 * 
 * Finds the next and previous segments relative to the input segment. 
 * It updates their pointers to remove the input segment from the linked list.
 * 
 * @param segment the segment to remove
 */
void removeHeapSegment(struct HeapSegment *segment)
{
	//get pointers to next and previous segments
	struct HeapSegment *nextSegment = segment->nextSegment;
	struct HeapSegment *prevSegment = segment->prevSegment;

	//if there is a previous segment, set its next segment to the next segment of the removed segment
	if (prevSegment)
	{
		prevSegment->nextSegment = nextSegment;
	}
	//else there is no previous segment to the first segment gets updated
	else
	{
		gFirstFreeSegment = nextSegment;
	}
	//if there is a next segment, set its previous segment to the previous segment of the removed segment
	if (nextSegment)
	{
		nextSegment->prevSegment = prevSegment;
	}
}

/**
 * @brief Inserts a segment into the linked list of free segments.
 * 
 * @param at where to insert the segment
 * @param segment the segment to insert
 */
void insertHeapSegment(struct HeapSegment *at, struct HeapSegment *segment)
{
	struct HeapSegment *nextSegment;
	struct HeapSegment *prevSegment;

	if (at)
	{
		nextSegment = at->nextSegment;
		prevSegment = at;
	}
	else
	{
		nextSegment = gFirstFreeSegment;
		prevSegment = 0;
	}

	segment->nextSegment = nextSegment;
	segment->prevSegment = prevSegment;

	if (nextSegment)
	{
		nextSegment->prevSegment = segment;
	}

	if (prevSegment)
	{
		prevSegment->nextSegment = segment;
	}
	else
	{
		gFirstFreeSegment = segment;
	}
}

struct HeapSegment *getPrevBlock(struct HeapSegment *at, int type)
{
	struct HeapSegmentFooter *prevFooter = (struct HeapSegmentFooter *)at - 1;

	if ((void *)prevFooter < gHeapStart || (void *)prevFooter >= gHeapEnd)
	{
		return 0;
	}

	if (prevFooter->footer != (MALLOC_BLOCK_FOOT | type))
	{
		return 0;
	}

	return prevFooter->header;
}

/**
 * @brief Get the Next Block object after the given adress
 * 
 * @param at The adress representing the current block
 * @param type The type of the block to get
 * @return struct HeapSegment* 
 */
struct HeapSegment *getNextBlock(struct HeapSegment *at, int type)
{
	//set adress of the next header to the end of the current segment
	struct HeapUsedSegment *nextHeader = (struct HeapUsedSegment *)at->segmentEnd;

	//check if the next block is in the bounds of the heap
	if ((void *)nextHeader < gHeapStart || (void *)nextHeader >= gHeapEnd)
	{
		return 0;
	}
	//check if the next block is a valid header
	if (nextHeader->header != (MALLOC_BLOCK_HEAD | type))
	{
		return 0;
	}
	//return the next block
	return (struct HeapSegment *)nextHeader;
}

void *malloc(unsigned int size)
{
	struct HeapSegment *currentSegment;
	int segmentSize;
	// 8 byte align for DMA
	size = (size + 7) & (~0x7);

	size += sizeof(struct HeapUsedSegment);
	size += sizeof(struct HeapSegmentFooter);

	currentSegment = gFirstFreeSegment;

	while (currentSegment)
	{
		segmentSize = (int)currentSegment->segmentEnd - (int)currentSegment;

		if (segmentSize >= size)
		{
			void *newEnd = currentSegment->segmentEnd;
			struct HeapUsedSegment *newSegment;
			if (segmentSize >= size + MIN_HEAP_BLOCK_SIZE)
			{
				newSegment = (struct HeapUsedSegment *)((char *)newEnd - size);

				struct HeapSegment *prevSeg = currentSegment->prevSegment;

				removeHeapSegment(currentSegment);
				heapInitBlock(currentSegment, newSegment, MALLOC_FREE_BLOCK);
				insertHeapSegment(prevSeg, currentSegment);
			}
			else
			{
				removeHeapSegment(currentSegment);

				newSegment = (struct HeapUsedSegment *)currentSegment;
			}

			heapInitBlock((struct HeapSegment *)newSegment, newEnd, MALLOC_USED_BLOCK);

			return newSegment + 1;
		}

		currentSegment = currentSegment->nextSegment;
	}

	return 0;
}

/**
 * @brief the given memory block to a new size.
 * 
 * If target is NULL, this simply calls malloc() to allocate a new block.
 * Otherwise, it will attempt to resize the existing block in-place if possible.
 * If the new size is larger, it may claim additional free space after the block if available.  
 * If insufficient space is available, we try to allocate a new block and copy the data over as last resort.
 * 
 * Returns the reallocated block pointer (unchanged if no resizing was possible).
 * 
 * @param target the memory block to resize
 * @param size the new size of the block
 */
void *realloc(void *target, unsigned int size)
{
	//if the target is NULL, we can't resize it, so we just call malloc()
	if (!target)
	{
		return malloc(size);
	}

	//get block metadata
	struct HeapUsedSegment *segment = (struct HeapUsedSegment *)target - 1;

	//calculate the new block end
	void *resizedBlockEnd = (char *)target + ALIGN_8(size) + sizeof(struct HeapSegmentFooter);

	//if the new size is smaller, we can just shrink the block
	if (resizedBlockEnd < segment->segmentEnd)
	{
		//get the next segment pointer
		struct HeapSegment *nextSegment = (struct HeapSegment *)resizedBlockEnd;
		//calculate the unused memory at the end of the block
		u32 unusedMemory = (char *)segment->segmentEnd - (char *)resizedBlockEnd;

		//if the unused memory is too small, we can't shrink the block and return the original block
		if (unusedMemory < MIN_HEAP_BLOCK_SIZE)
		{
			return target;
		}

		//otherwise init the the block that should be freed and the rezized block
		heapInitBlock(nextSegment, segment->segmentEnd, MALLOC_USED_BLOCK);
		heapInitBlock((struct HeapSegment *)segment, resizedBlockEnd, MALLOC_USED_BLOCK);
		//free the unused memory
		free(nextSegment);
	}
	//if the new size is larger, we can try to claim additional free space after the block if available
	else if (resizedBlockEnd > segment->segmentEnd)
	{
		//try to get the next free block
		struct HeapSegment *followingFreeSegment = getNextBlock((struct HeapSegment *)segment, MALLOC_FREE_BLOCK);

		//check if a free block was found
		if (followingFreeSegment)
		{
			//if the end of the resized block is smaller than the end of the next free block we can resize into the free space
			if (resizedBlockEnd < followingFreeSegment->segmentEnd)
			{
				//get the adress of the free segment prior to the free one after the block that is about to be rezised
				struct HeapSegment *prevFreeSegment = followingFreeSegment->prevSegment;
				//remove the free segment from the linked list of free segments
				removeHeapSegment(followingFreeSegment);

				//check if the additionally needed memory is big enough to be a valid free segment
				if (((char *)followingFreeSegment->segmentEnd - (char *)resizedBlockEnd) >= MIN_HEAP_BLOCK_SIZE)
				{
					struct HeapSegment *newSegment = (struct HeapSegment *)resizedBlockEnd;
					//make the remaining memory a free segment
					heapInitBlock(newSegment, followingFreeSegment->segmentEnd, MALLOC_FREE_BLOCK);
					//init the segment from the start of the original segment to the end of the resized segment
					heapInitBlock((struct HeapSegment *)segment, resizedBlockEnd, MALLOC_USED_BLOCK);

					//insert the remaining free segment into the linked list of free segments
					insertHeapSegment(prevFreeSegment, newSegment);
				}
				//otherwise use the entire free following block for the resize
				else
				{
					heapInitBlock((struct HeapSegment *)segment, followingFreeSegment->segmentEnd, MALLOC_USED_BLOCK);
				}

				return target;
			}
			//the free space following the existing block is too small, as last resort try to a new block for the entire thing
			else{
				void *result = malloc(size);
				memCopy(result, target, size);
				free(target);
				return result;
			}
		}
		//there is no free segment following the existing block, so we have to allocate a new block for the entire thing
		else{
			void *result = malloc(size);
			memCopy(result, target, size);
			free(target);
			return result;
		}
	}
	//the new size is the same as the old size, so we can just return the original block
	return target;
}

/**
 * @brief release memory allocated by malloc()
 * 
 * @param target pointer to the memory block to be freed
 */
void free(void *target)
{
	//check if the target is inside the heap boundaries
	if ((void *)target < gHeapStart || (void *)target >= gHeapEnd)
	{
		return;
	}
	//get target Segment Metadata
	struct HeapUsedSegment *segment = (struct HeapUsedSegment *)target - 1;

	struct HeapSegment *prev = getPrevBlock((struct HeapSegment *)segment, MALLOC_FREE_BLOCK);
	struct HeapSegment *next = getNextBlock((struct HeapSegment *)segment, MALLOC_FREE_BLOCK);

	void *segmentEnd = segment->segmentEnd;

	//if there is a previous free segment, merge it by removing the prev and updating the start adress
	if (prev)
	{
		segment = (struct HeapUsedSegment *)prev;
		removeHeapSegment(prev);
	}
	//if there is a following free segment, merge it by removing the following and updating the end adress
	if (next)
	{
		segmentEnd = next->segmentEnd;
		removeHeapSegment(next);
	}
	//init the segment as free block with potentially updated adresses
	heapInitBlock((struct HeapSegment *)segment, segmentEnd, MALLOC_FREE_BLOCK);
	//insert the now free block at the beginning of the linked list of free blocks
	insertHeapSegment(0, (struct HeapSegment *)segment);
}

int calculateHeapSize()
{
	return (char *)gHeapEnd - (char *)gHeapStart;
}

int calculateBytesFree()
{
	int result;
	struct HeapSegment *currentSegment;

	currentSegment = gFirstFreeSegment;
	result = 0;

	while (currentSegment != 0)
	{
		result += (int)currentSegment->segmentEnd - (int)currentSegment;
		currentSegment = currentSegment->nextSegment;
	}

	return result;
}

int calculateLargestFreeChunk()
{
	int result;
	int current;
	struct HeapSegment *currentSegment;

	currentSegment = gFirstFreeSegment;
	result = 0;

	while (currentSegment != 0)
	{
		current = (int)currentSegment->segmentEnd - (int)currentSegment;

		if (current > result)
		{
			result = current;
		}

		currentSegment = currentSegment->nextSegment;
	}

	return result;
}

void zeroMemory(void *memory, int size)
{
	unsigned char *asChar = (unsigned char *)memory;

	while (size > 0)
	{
		*asChar = 0;
		++asChar;
		--size;
	}
}

/**
 * @brief copy the memory of specific size to another location
 * 
 * @param target the target adress
 * @param src the source adress
 * @param size the size
 */
void memCopy(void *target, const void *src, int size)
{
	unsigned char *targetAsChar;
	unsigned char *srcAsChar;

	targetAsChar = (unsigned char *)target;
	srcAsChar = (unsigned char *)src;

	while (size > 0)
	{
		*targetAsChar = *srcAsChar;
		++targetAsChar;
		++srcAsChar;
		--size;
	}
}

#define STACK_MALLOC_SIZE_BYTES (8 * 1024)
#define STACK_MALLOC_SIZE_WORDS (STACK_MALLOC_SIZE_BYTES >> 3)

int gStackMallocAt;
long long gStackMalloc[STACK_MALLOC_SIZE_WORDS];

void stackMallocReset()
{
	gStackMallocAt = 0;
}

void stackMallocFree(void *ptr)
{
	void *currentHead = &gStackMalloc[gStackMallocAt];

	if (ptr < currentHead)
	{
		gStackMallocAt = (long long *)ptr - gStackMalloc;
	}
}

void *stackMalloc(int size)
{
	int nWords = (size + 7) >> 3;
	void *result = &gStackMalloc[gStackMallocAt];
	gStackMallocAt += nWords;
	return result;
}
/*
 * Malloc implementations
 * September 26, 2017
 * Joseph A. Boyle
 * Rutgers, The State University of New Jersey
 */

/*
 * All of these implementations are written for architectures assuming integers are >= 4 bytes.
 */

#include <stdio.h>
#include <unistd.h>
#include "malloc.h"

#ifndef MALLOC_INITIAL_REQUEST_SIZE
	#define MALLOC_INITIAL_REQUEST_SIZE 4096
#endif

#define MALLOC_MAX_REQUEST_SIZE MALLOC_INITIAL_REQUEST_SIZE
#define MALLOC_BYTES_BEFORE_BLOCK 4
#define MALLOC_BYTES_AFTER_BLOCK 4
#define MALLOC_BYTES_METADATA_PER_BLOCK (MALLOC_BYTES_BEFORE_BLOCK + MALLOC_BYTES_AFTER_BLOCK)
#define MALLOC_FREE 0
#define MALLOC_INUSE 1

// A macro to get the number of real bytes a block will occupy, after accounting for the metadata it has.
// For example, if given 64, it will return 64 - MALLOC_BYTES_METADATA_PER_BLOCK
// This expects arguments as follows:
// N: Number of bytes
#define useableBytes(N) ((N) - MALLOC_BYTES_METADATA_PER_BLOCK)

// Internal data used in the malloc implementations.
uchar *data = NULL;

void* mymalloc(uint size){
	uint isFree, blockSize, metadata;
	// We begin by sbrk'ing our memory region if that has not yet happened.
	// In this naive implementation, we will not ever request more memory after we've filled our banks.
	// Thus, this is all we get.
	if(data == NULL){
		data = sbrk(MALLOC_INITIAL_REQUEST_SIZE);
		printf("Data pointer returned by sbrk: %p\n", data);
		if(data == NULL || data == 0){
			printf("SBRK RETURNED NULL!\n");
		}
		setData(data, useableBytes(MALLOC_INITIAL_REQUEST_SIZE), MALLOC_FREE);
	}

	// This request is bigger than the amount of memory we can allocate, so we failed to give it out.
	if(size > MALLOC_MAX_REQUEST_SIZE) return NULL;

	uchar *ptr = data;
	// We check that we haven't exceeded our boundaries for this memory block, which exist at
	// (data + MALLOC_INITIAL_REQUEST_SIZE) - 1
	while( ptr < (data + MALLOC_INITIAL_REQUEST_SIZE - 1) ){
		printf("Currently examining the block at: %p\n", ptr);
		metadata = readUint(ptr);
		isFree = (metadata >> 31);
		blockSize = (metadata & 0x7FFFFFFF);
		printf("The block is of size %d and free status: %d\n", blockSize, isFree);
		if(blockSize >= size){
			printf("This block is large enough for our request\n");
			// First, we decide if there will be enough bytes after we are finished such that we can even allocate another block after.
			// If we cannot, then we will absorb this whole block for this request
			// This occurs when the difference between the blocksize and the request is > 0 && <= 8 bytes.
			// Reasoning for above: if the difference is 0, we take up the full block.
			// If the difference is 9 or more, then we can fit 8 bytes of metadata + 1 block of space.
			if((blockSize - size) >= 0 && (blockSize - size) <= 8){
				printf("This block isn't big enough to be used after this request, so we're giving you all of its space.\n");
				// Set these bytes to inuse, and return the first useable address.
				setData(ptr, blockSize, MALLOC_INUSE);
				return (void*) (ptr + 4);
			}else{
				printf("There's enough room in this block to create a free block of %d bytes after your request.\n", useableBytes(blockSize-size));
				// Set size bytes to inuse
				setData(ptr, size, MALLOC_INUSE);
				// Then set the chunk after this
				setData(ptr + size + MALLOC_BYTES_METADATA_PER_BLOCK, useableBytes(blockSize - size), MALLOC_FREE);
				return (void*) (ptr + 4);
			}
		}
		ptr += blockSize + MALLOC_BYTES_METADATA_PER_BLOCK;
	}

	return NULL;
}

// define the data for a block.
// This expects arguments as follows:
// address: (of where to place the first metadata block),
// size: (number of bytes that the block actually possesses for usage).
// free: (MALLOC_INUSE for in use, or MALLOC_FREE for free).
void setData(void* address, uint size, int free){
	uint* p = (uint*) address;
	printf("Setting data at %p to: %0x\n", p, (free << 31) | (size & 0x79FFFFFF));
	(*p) = (free << 31) | (size & 0x09FFFFFF);
	p = (uint*) (((uchar*) address) + (MALLOC_BYTES_BEFORE_BLOCK + size));
	printf("Setting data at %p to: %0x\n", p, (free << 31) | (size & 0x79FFFFFF));
	(*p) = (free << 31) | (size & 0x09FFFFFF);
}

// Reads a uint from the specified address
// Expects the address at which we start this.
uint readUint(void* address){
	uint* p = (uint*) address;
	return (*p);
}

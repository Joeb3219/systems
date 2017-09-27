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

#define DEBUG 0

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
	if(DEBUG) printf("[Call] Being asked to malloc %d bytes\n", size);
	uint isFree, blockSize, metadata;
	// We begin by sbrk'ing our memory region if that has not yet happened.
	// In this naive implementation, we will not ever request more memory after we've filled our banks.
	// Thus, this is all we get.
	if(data == NULL){
		data = sbrk(MALLOC_INITIAL_REQUEST_SIZE);
		if(data == NULL || data == 0){
			return NULL;
		}
		setData(data, useableBytes(MALLOC_INITIAL_REQUEST_SIZE), MALLOC_FREE);
	}

	// This request is bigger than the amount of memory we can allocate, so we failed to give it out.
	if(size > MALLOC_MAX_REQUEST_SIZE) return NULL;

	uchar *ptr = data;
	// We check that we haven't exceeded our boundaries for this memory block, which exist at
	// (data + MALLOC_INITIAL_REQUEST_SIZE) - 1
	while( ptr < (data + MALLOC_INITIAL_REQUEST_SIZE - 1) ){
		metadata = readUint(ptr);
		isFree = (metadata >> 31);
		blockSize = (metadata & 0x7FFFFFFF);
		if(blockSize >= size){
			// First, we decide if there will be enough bytes after we are finished such that we can even allocate another block after.
			// If we cannot, then we will absorb this whole block for this request
			// This occurs when the difference between the blocksize and the request is > 0 && <= 8 bytes.
			// Reasoning for above: if the difference is 0, we take up the full block.
			// If the difference is 9 or more, then we can fit 8 bytes of metadata + 1 block of space.
			if((blockSize - size) >= 0 && (blockSize - size) <= 8){
				// Set these bytes to inuse, and return the first useable address.
				setData(ptr, blockSize, MALLOC_INUSE);
				return (void*) (ptr + 4);
			}else{
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

// Frees some data that we previously malloc'd via mymalloc.
// It is pertinent that the pointer is exactly as returned by mymalloc.
// 4 bytes before ptr will be the metadata we need to free up this space.
void myfree(void* ptr){
	if(DEBUG) printf("[Call] Being asked to free %p\n", ptr);
	// We move back 4 bytes such that we are now at the metadata portion.
	ptr = (void*) (((char*) ptr) - 4);
	uint metadata, isFree, blockSize;
	metadata = readUint(ptr);
	isFree = (metadata >> 31);
	blockSize = (metadata & 0x7FFFFFFF);

	if(DEBUG) printf("This block is of size: %d\n", blockSize);

	// At a minimum, we set this block to free.
	setData(ptr, blockSize, MALLOC_FREE);

	// Now we attempt to merge this block into the block above it, if it exists.
	void* nextBlock = (void*) (((char*)ptr) + blockSize + MALLOC_BYTES_METADATA_PER_BLOCK);
	if( ((uchar*) nextBlock) < (data + MALLOC_INITIAL_REQUEST_SIZE - 1) ){
		if(DEBUG) printf("Examining the block above the currently free'd block, at %p\n", nextBlock);
		// Grab the metadata of this block so that we can see if it is free.
		metadata = readUint(nextBlock);
		if( (metadata >> 31) == MALLOC_FREE){
			// This block is also free, so we can perform a merge up.
			// The new size will gain 8 bytes (the old endtag of this block + the old start block of the next block) + how ever big the new block is.
			blockSize += (metadata & 0x7FFFFFFF) + 8;
			setData(ptr, blockSize, MALLOC_FREE);
		}
	}

	// Now we attempt to merge this block into the block below it, if it exists.
	// Since directly below our ptr is MALLOC_BYTES_AFTER_BLOCK of metadata, we can move down that
	// many bytes.
	// We can check if this tag is where our data line ends and if not attempt to do a merge.
	nextBlock = (void*) ( ((char*) ptr) - MALLOC_BYTES_AFTER_BLOCK);
	if( ((uchar*) nextBlock) > data ){
		if(DEBUG) printf("Examining the block below the currently free'd block, at %p\n", nextBlock);
		// Grab the metadata of this block so that we can see if it is free.
		metadata = readUint(nextBlock);
		if( (metadata >> 31) == MALLOC_FREE){
			// This block is also free, so we can perform a merge up.
			// The new size will gain 8 bytes (the old endtag of this block + the old start block of the next block) + how ever big the new block is.
			blockSize += (metadata & 0x7FFFFFFF) + 8;
			// The actual block we're going to be starting at is the start of this block.
			// We are currently looking at: |META|DATA|META|, where we are in the last section.
			// So we have to move our nextBlock back the size of the data + 1 meta.
			nextBlock = (void*) ( ((uchar*) nextBlock) - MALLOC_BYTES_BEFORE_BLOCK - (metadata & 0x7FFFFFFF) );
			setData(nextBlock, blockSize, MALLOC_FREE);
		}
	}

}

// define the data for a block.
// This expects arguments as follows:
// address: (of where to place the first metadata block),
// size: (number of bytes that the block actually possesses for usage).
// free: (MALLOC_INUSE for in use, or MALLOC_FREE for free).
void setData(void* address, uint size, int free){
	if(DEBUG){
		if(free == MALLOC_FREE) printf("Setting %d blocks at %p to free.\n", size, address);
		else printf("Setting %d blocks at %p to inuse.\n", size, address);
	}
	uint* p = (uint*) address;
	(*p) = (free << 31) | (size & 0x09FFFFFF);
	p = (uint*) (((uchar*) address) + (MALLOC_BYTES_BEFORE_BLOCK + size));
	(*p) = (free << 31) | (size & 0x09FFFFFF);
}

// Reads a uint from the specified address
// Expects the address at which we start this.
uint readUint(void* address){
	uint* p = (uint*) address;
	return (*p);
}

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
#define useableBytes(N) (N - MALLOC_BYTES_METADATA_PER_BLOCK)

// Internal data used in the malloc implementations.
uchar *data = NULL;

void* mymalloc(uint size){
	// We begin by sbrk'ing our memory region if that has not yet happened.
	// In this naive implementation, we will not ever request more memory after we've filled our banks.
	// Thus, this is all we get.
	if(data == NULL){
		data = sbrk(MALLOC_INITIAL_REQUEST_SIZE);
		printf("Data pointer returned by sbrk: %p\n", data);
		if(data == NULL || data == 0){
			printf("SBRK RETURNED NULL!\n");
		}
		setData(0, useableBytes(MALLOC_INITIAL_REQUEST_SIZE), MALLOC_FREE);
	}

	// This request is bigger than the amount of memory we can allocate, so we failed to give it out.
	if(size > MALLOC_MAX_REQUEST_SIZE) return NULL;
	return NULL;
}

// define the data for a block.
// This expects arguments as follows:
// address: (of where to place the first metadata block),
// size: (number of bytes that the block actually possesses for usage).
// free: (MALLOC_INUSE for in use, or MALLOC_FREE for free).
void setData(uint address, uint size, int free){
	int* p = (int*) (&data[address]);
	printf("Setting data at %p (%d past %p)\n", p, address, data);
	(*p) = (free << 31) | (size & 0x09FFFFFF);
	(int*) &data[address + MALLOC_BYTES_BEFORE_BLOCK + size];
	(*p) = (free << 31) | (size & 0x09FFFFFF);
}

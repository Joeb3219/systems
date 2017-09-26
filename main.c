#include <stdio.h>
#include "malloc/malloc.h"

int main(int argc, char** argv){

	// Testing for naiive malloc
	printf("Testing naiive malloc (begun 9/26/17):\n");

	int* mem = mymalloc(32);
	int* mem2 = mymalloc(64);

	printf("Mem exists at %p\n", mem);
	printf("Mem2 exists at %p\n", mem2);

	printf("End testing naiive malloc.\n\n\n");
}

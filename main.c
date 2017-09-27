#include <stdio.h>
#include "malloc/malloc.h"

int main(int argc, char** argv){

	// Testing for naiive malloc
	printf("Testing naiive malloc (begun 9/26/17):\n");

	int* mem = mymalloc(32);
	int* mem2 = mymalloc(64);

	printf("Mem exists at %p\n", mem);
	printf("Mem2 exists at %p\n", mem2);

	myfree(mem);
	mem = mymalloc(18);
	printf("Mem exists at %p\n", mem);
	int* test = mymalloc(64);
	myfree(mem2);
	int* test2 = mymalloc(18);

	printf("Test exists at %p\n", test);
	printf("Test2 exists at %p\n", test2);

	printf("End testing naiive malloc.\n\n\n");
}

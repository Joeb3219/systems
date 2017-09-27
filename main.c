#include <stdio.h>
#include "malloc/malloc.h"

#define malloc(A) mymalloc(A)
#define free(A) myfree(A)

int main(int argc, char** argv){

	// Testing for naiive malloc
	printf("Testing naiive malloc (begun 9/26/17):\n");

	int* mem = malloc(32);
	int* mem2 = malloc(64);

	printf("Mem exists at %p\n", mem);
	printf("Mem2 exists at %p\n", mem2);

	free(mem);
	mem = malloc(18);
	printf("Mem exists at %p\n", mem);
	int* test = malloc(64);
	free(mem2);
	int* test2 = malloc(18);

	printf("Test exists at %p\n", test);
	printf("Test2 exists at %p\n", test2);

	free(test2);
	free(test);
	free(mem);

	printf("End testing naiive malloc.\n\n\n");
}

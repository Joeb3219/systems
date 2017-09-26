#include <stdio.h>
#include "malloc/malloc.h"

int main(int argc, char** argv){

	// Testing for naiive malloc
	printf("Testing naiive malloc (begun 9/26/17):\n");

	int* mem = mymalloc(32);

	printf("End testing naiive malloc.\n\n\n");
}

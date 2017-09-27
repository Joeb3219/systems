#ifndef MALLOC_H_

	#define MALLOC_H_

	typedef unsigned int uint;
	typedef unsigned char uchar;

	void* mymalloc(uint size);
	void myfree(void* ptr);
	void setData(void* address, uint size, int free);
	uint readUint(void* address);
#endif

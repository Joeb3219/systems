#ifndef MALLOC_H_

	#define MALLOC_H_

	typedef unsigned int uint;
	typedef unsigned char uchar;

	void* mymalloc(uint size);
	void setData(uint address, uint size, int free);
#endif

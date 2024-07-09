#ifndef _LINKER_ALLOC_H_
#define _LINKER_ALLOC_H_ 1
#include <sys/types.h>



void* linker_alloc(u64 size);



void linker_dealloc(void* ptr);



#endif

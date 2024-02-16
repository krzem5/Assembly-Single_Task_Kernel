#ifndef _LINKER_ALLOC_H_
#define _LINKER_ALLOC_H_ 1
#include <sys/types.h>



typedef void* (*allocator_backend_t)(void* null,void* ptr,u64 size);



void* alloc(u64 size);



void dealloc(void* ptr);



void alloc_change_backend(allocator_backend_t backend);



#endif

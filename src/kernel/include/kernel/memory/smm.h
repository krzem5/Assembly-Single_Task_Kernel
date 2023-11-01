#ifndef _KERNEL_MEMORY_SMM_H_
#define _KERNEL_MEMORY_SMM_H_ 1
#include <kernel/types.h>



#define SMM_MAX_LENGTH 255



typedef struct _STRING{
	u32 length;
	u32 hash;
	char data[];
} string_t;



string_t* smm_alloc(const char* name,u32 length);



void smm_dealloc(string_t* name);



string_t* smm_duplicate(const string_t* name);



void smm_rehash(string_t* name);



#endif

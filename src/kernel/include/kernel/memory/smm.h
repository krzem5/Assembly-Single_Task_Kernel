#ifndef _KERNEL_MEMORY_SMM_H_
#define _KERNEL_MEMORY_SMM_H_ 1
#include <kernel/types.h>



#define SMM_MAX_LENGTH 255

#define SMM_TEMPORARY_STRING string_t* __attribute__((cleanup(_smm_cleanup)))



typedef struct _STRING{
	u32 length;
	u32 hash;
	char data[];
} string_t;



string_t* smm_alloc(const char* data,u32 length);



void smm_dealloc(string_t* string);



string_t* smm_duplicate(const string_t* string);



void smm_rehash(string_t* string);



u32 smm_length(const char* data);



void _smm_cleanup(string_t** string);



#endif

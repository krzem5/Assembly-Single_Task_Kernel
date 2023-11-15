#ifndef _CORE_MEMORY_H_
#define _CORE_MEMORY_H_ 1
#include <core/types.h>



#define MEMORY_FLAG_READ 1
#define MEMORY_FLAG_WRITE 2
#define MEMORY_FLAG_EXEC 4
#define MEMORY_FLAG_FILE 8
#define MEMORY_FLAG_NOWRITEBACK 16



void* memory_map(u64 length,u32 flags,u64 fd);



_Bool memory_change_flags(void* address,u64 length,u32 flags);



_Bool memory_unmap(void* address,u64 length);



#endif

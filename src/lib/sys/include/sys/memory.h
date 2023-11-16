#ifndef _SYS_MEMORY_H_
#define _SYS_MEMORY_H_ 1
#include <sys/types.h>



#define SYS_MEMORY_FLAG_READ 1
#define SYS_MEMORY_FLAG_WRITE 2
#define SYS_MEMORY_FLAG_EXEC 4
#define SYS_MEMORY_FLAG_FILE 8
#define SYS_MEMORY_FLAG_NOWRITEBACK 16



void* sys_memory_map(u64 length,u32 flags,u64 fd);



_Bool sys_memory_change_flags(void* address,u64 length,u32 flags);



_Bool sys_memory_unmap(void* address,u64 length);



#endif

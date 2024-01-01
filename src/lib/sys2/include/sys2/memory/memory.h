#ifndef _SYS2_MEMORY_MEMORY_H_
#define _SYS2_MEMORY_MEMORY_H_ 1
#include <sys2/error/error.h>
#include <sys2/fd/fd.h>
#include <sys2/types.h>



#define SYS2_PAGE_SIZE 4096

#define SYS2_MEMORY_FLAG_READ 1
#define SYS2_MEMORY_FLAG_WRITE 2
#define SYS2_MEMORY_FLAG_EXEC 4
#define SYS2_MEMORY_FLAG_FILE 8
#define SYS2_MEMORY_FLAG_NOWRITEBACK 16



sys2_error_t sys2_memory_map(u64 length,u32 flags,sys2_fd_t fd);



sys2_error_t sys2_memory_change_flags(void* address,u64 length,u32 flags);



sys2_error_t sys2_memory_unmap(void* address,u64 length);



#endif

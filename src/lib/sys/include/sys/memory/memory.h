#ifndef _SYS_MEMORY_MEMORY_H_
#define _SYS_MEMORY_MEMORY_H_ 1
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/types.h>



#define SYS_PAGE_SIZE 4096

#define SYS_MEMORY_FLAG_READ 1
#define SYS_MEMORY_FLAG_WRITE 2
#define SYS_MEMORY_FLAG_EXEC 4
#define SYS_MEMORY_FLAG_FILE 8
#define SYS_MEMORY_FLAG_NOWRITEBACK 16
#define SYS_MEMORY_FLAG_STACK 32



static inline u64 sys_memory_align_up_address(u64 base){
	return (base+SYS_PAGE_SIZE-1)&(-SYS_PAGE_SIZE);
}



static inline u64 sys_memory_align_down_address(u64 base){
	return base&(-SYS_PAGE_SIZE);
}



u64 sys_memory_map(u64 length,u32 flags,sys_fd_t fd);



sys_error_t __attribute__((nonnull)) sys_memory_change_flags(void* address,u64 length,u32 flags);



sys_error_t __attribute__((nonnull)) sys_memory_unmap(void* address,u64 length);



void __attribute__((access(read_only,1,3),access(write_only,2,3))) sys_memory_copy(const void* src,void* dst,u64 length);



s32 __attribute__((access(read_only,1,3),access(read_only,2,3))) sys_memory_compare(const void* a,const void* b,u64 length);



void __attribute__((access(write_only,1,2))) sys_memory_set(void* dst,u64 length,u8 value);



#endif

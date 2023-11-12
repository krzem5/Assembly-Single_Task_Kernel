#ifndef _USER_SYSCALL_H_
#define _USER_SYSCALL_H_ 1



typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;



void _syscall_invalid(void);



void _syscall_clock_get_converion(void);



u32 _syscall_cpu_get_count(void);



s64 _syscall_fd_close(u64 fd);



s64 _syscall_fd_dup(u64 fd,u32 flags);



s64 _syscall_fd_iter_get(u64 iterator,char* buffer,u32 size);



s64 _syscall_fd_iter_next(u64 iterator);



s64 _syscall_fd_iter_start(u64 fd);



s64 _syscall_fd_iter_stop(u64 iterator);



s64 _syscall_fd_open(u64 fd,const char* path,u32 length,u32 flags);



s64 _syscall_fd_path(u64 fd,char* buffer,u32 size);



s64 _syscall_fd_read(u64 fd,void* buffer,u64 size,u32 flags);



s64 _syscall_fd_resize(u64 fd,u64 size,u32 flags);



s64 _syscall_fd_seek(u64 fd,u64 offset,u32 type);



s64 _syscall_fd_stat(u64 fd,void* ptr,u32 size);



s64 _syscall_fd_write(u64 fd,const void* buffer,u64 size,u32 flags);



void* _syscall_memory_map(u64 length,u32 flags);



_Bool _syscall_memory_unmap(void* address,u64 length);



void __attribute__((noreturn)) _syscall_system_shutdown(u32 flags);



u64 _syscall_thread_create(u64 rip,u64 rdi,u64 rsi,u64 stack_size);



_Bool _syscall_thread_get_cpu_mask(u64 handle,void* buffer,u32 size);



u32 _syscall_thread_get_priority(u64 handle);



_Bool _syscall_thread_set_cpu_mask(u64 handle,void* buffer,u32 size);



_Bool _syscall_thread_set_priority(u64 handle,u32 priority);



void __attribute__((noreturn)) _syscall_thread_stop(void);



#endif

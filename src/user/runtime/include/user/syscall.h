#ifndef _USER_SYSCALL_H_
#define _USER_SYSCALL_H_ 1
#include <user/types.h>



void _syscall_print_string(const char* str,u32 length);



s32 _syscall_elf_load(const char* path,u32 length);



u32 _syscall_cpu_core_count(void);



s32 _syscall_cpu_core_start(u32 index,void* fn,void* arg);



void _syscall_cpu_core_stop(void);



u32 _syscall_drive_list_length(void);



s32 _syscall_drive_list_get(u32 index,void* ptr,u32 size);



u32 _syscall_file_system_count(void);



s32 _syscall_file_system_get(u32 index,void* ptr,u32 size);



s32 _syscall_fd_open(const char* path,u32 length,u32 flags);



s32 _syscall_fd_close(s32 fd);



s32 _syscall_fd_delete(s32 fd);



s32 _syscall_fd_read(s32 fd,void* buffer,u32 size);



s32 _syscall_fd_write(s32 fd,const void* buffer,u32 size);



s64 _syscall_fd_seek(s32 fd,u64 offset,u32 type);



s32 _syscall_fd_stat(s32 fd,void* ptr,u32 size);



s32 _syscall_fd_get_relative(s32 fd,u32 relative);



s32 _syscall_fd_dup(s32 fd,u32 flags);



_Bool _syscall_net_send(const void* packet,u32 length);



_Bool _syscall_net_poll(void* packet,u32 length);



void _syscall_acpi_shutdown(_Bool restart);



#endif

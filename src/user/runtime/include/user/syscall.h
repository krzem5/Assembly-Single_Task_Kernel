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



s32 _syscall_fd_open(u32 root,const char* path,u32 length,u32 flags);



s32 _syscall_fd_close(u32 fd);



s32 _syscall_fd_delete(u32 fd);



s32 _syscall_fd_read(u32 fd,void* buffer,u32 size);



s32 _syscall_fd_write(u32 fd,const void* buffer,u32 size);



s64 _syscall_fd_seek(u32 fd,u64 offset,u32 type);



s32 _syscall_fd_stat(u32 fd,void* ptr,u32 size);



s32 _syscall_fd_get_relative(u32 fd,u32 relative,void* ptr,u32 size);



s32 _syscall_fd_dup(u32 fd);



s32 _syscall_net_send(const void* buffer,u32 length);



s32 _syscall_net_poll(void* buffer,u32 length);



void _syscall_acpi_shutdown(_Bool restart);



#endif

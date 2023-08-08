#ifndef _USER_SYSCALL_H_
#define _USER_SYSCALL_H_ 1
#include <user/types.h>



void _syscall_serial_send(const void* buffer,u32 length);



u32 _syscall_serial_recv(void* buffer,u32 length,u64 timeout);



s32 _syscall_elf_load(const char* path,u32 length);



u32 _syscall_cpu_core_count(void);



s32 _syscall_cpu_core_start(u32 index,void* fn,void* arg1,void* arg2);



void _syscall_cpu_core_stop(void);



u32 _syscall_drive_list_length(void);



s32 _syscall_drive_list_get(u32 index,void* ptr,u32 size);



u32 _syscall_partition_count(void);



s32 _syscall_partition_get(u32 index,void* ptr,u32 size);



s32 _syscall_fd_open(s32 fd,const char* path,u32 length,u32 flags);



s32 _syscall_fd_close(s32 fd);



s32 _syscall_fd_delete(s32 fd);



s32 _syscall_fd_read(s32 fd,void* buffer,u32 size);



s32 _syscall_fd_write(s32 fd,const void* buffer,u32 size);



s64 _syscall_fd_seek(s32 fd,u64 offset,u32 type);



s32 _syscall_fd_stat(s32 fd,void* ptr,u32 size);



s32 _syscall_fd_get_relative(s32 fd,u32 relative,u32 flags);



s32 _syscall_fd_move(s32 fd,u32 dst_fd);



_Bool _syscall_network_layer1_config(void* buffer,u32 length);



_Bool _syscall_network_layer2_send(const void* packet,u32 length);



_Bool _syscall_network_layer2_poll(void* packet,u32 length);



void _syscall_system_shutdown(u32 flags);



_Bool _syscall_system_config(void* buffer,u32 size);



void* _syscall_memory_map(u64 length,u32 flags);



_Bool _syscall_memory_unmap(void* address,u64 length);



_Bool _syscall_memory_stats(void* buffer,u32 size);



void _syscall_clock_get_converion(void);



_Bool _syscall_drive_format(u32 index,const void* boot,u32 boot_length);



_Bool _syscall_drive_stats(u32 index,void* buffer,u32 size);



void _syscall_network_layer3_refresh(void);



#endif

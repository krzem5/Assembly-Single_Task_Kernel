#ifndef _USER_SYSCALL_H_
#define _USER_SYSCALL_H_ 1
#include <user/types.h>



void _syscall_empty(void);



void _syscall_serial_send(const void* buffer,u32 length);



u32 _syscall_serial_recv(void* buffer,u32 length,u64 timeout);



_Bool _syscall_elf_load(const char* path,u32 length);



s32 _syscall_fd_open(s32 fd,const char* path,u32 length,u32 flags);



s32 _syscall_fd_close(s32 fd);



s32 _syscall_fd_delete(s32 fd);



s32 _syscall_fd_read(s32 fd,void* buffer,u32 size);



s32 _syscall_fd_write(s32 fd,const void* buffer,u32 size);



s64 _syscall_fd_seek(s32 fd,u64 offset,u32 type);



s32 _syscall_fd_resize(s32 fd,u64 size);



s32 _syscall_fd_absolute_path(s32 fd,char* buffer,u32 buffer_length);



s32 _syscall_fd_stat(s32 fd,void* ptr,u32 size);



s32 _syscall_fd_get_relative(s32 fd,u32 relative,u32 flags);



s32 _syscall_fd_move(s32 fd,u32 dst_fd);



_Bool _syscall_network_layer1_config(void* buffer,u32 length);



_Bool _syscall_network_layer2_send(const void* packet,u32 length);



_Bool _syscall_network_layer2_poll(void* packet,u32 length,_Bool block);



void _syscall_network_layer3_refresh(void);



u32 _syscall_network_layer3_device_count(void);



_Bool _syscall_network_layer3_device_get(u32 index,void* buffer,u32 size);



_Bool _syscall_network_layer3_device_delete(const void* buffer,u32 size);



void _syscall_system_shutdown(u32 flags);



void* _syscall_memory_map(u64 length,u32 flags);



_Bool _syscall_memory_unmap(void* address,u64 length);



u32 _syscall_memory_get_counter_count(void);



_Bool _syscall_memory_get_counter(u32 counter,void* buffer,u32 size);



u32 _syscall_memory_get_object_counter_count(void);



_Bool _syscall_memory_get_object_counter(u32 counter,void* buffer,u32 size);



void _syscall_clock_get_converion(void);



_Bool _syscall_drive_format(u32 index,const void* boot,u32 boot_length);



_Bool _syscall_drive_stats(u32 index,void* buffer,u32 size);



_Bool _syscall_random_generate(void* buffer,u64 size);



const void* _syscall_user_data_pointer(void);



void __attribute__((noreturn)) _syscall_thread_stop(void);



u64 _syscall_thread_create(u64 rip,u64 rdi,u64 rsi,u64 stack_size);



u32 _syscall_thread_get_priority(u64 handle);



_Bool _syscall_thread_set_priority(u64 handle,u32 priority);



u32 _syscall_handle_get_type_count(void);



_Bool _syscall_handle_get_type(u32 handle_type,void* buffer,u32 size);



_Bool _syscall_scheduler_get_stats(u32 cpu_index,void* buffer,u32 size);



_Bool _syscall_scheduler_get_timers(u32 cpu_index,void* buffer,u32 size);



_Bool _syscall_thread_get_cpu_mask(u64 handle,void* buffer,u32 size);



_Bool _syscall_thread_set_cpu_mask(u64 handle,void* buffer,u32 size);



#endif

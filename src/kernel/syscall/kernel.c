#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



extern u64 syscall_acl_get_permissions();
extern u64 syscall_acl_request_permissions();
extern u64 syscall_acl_set_permissions();
extern u64 syscall_clock_get_converion();
extern u64 syscall_cpu_get_count();
extern u64 syscall_drive_get_data();
extern u64 syscall_drive_get_next();
extern u64 syscall_event_create();
extern u64 syscall_event_delete();
extern u64 syscall_event_dispatch();
extern u64 syscall_event_set_active();
extern u64 syscall_fd_close();
extern u64 syscall_fd_dup();
extern u64 syscall_fd_iter_get();
extern u64 syscall_fd_iter_next();
extern u64 syscall_fd_iter_start();
extern u64 syscall_fd_iter_stop();
extern u64 syscall_fd_open();
extern u64 syscall_fd_path();
extern u64 syscall_fd_read();
extern u64 syscall_fd_resize();
extern u64 syscall_fd_seek();
extern u64 syscall_fd_stat();
extern u64 syscall_fd_write();
extern u64 syscall_fs_descriptor_get_data();
extern u64 syscall_fs_descriptor_get_next();
extern u64 syscall_fs_get_data();
extern u64 syscall_fs_get_next();
extern u64 syscall_fs_mount();
extern u64 syscall_gid_get();
extern u64 syscall_gid_get_name();
extern u64 syscall_gid_set();
extern u64 syscall_memory_change_flags();
extern u64 syscall_memory_get_size();
extern u64 syscall_memory_map();
extern u64 syscall_memory_unmap();
extern u64 syscall_partition_get_data();
extern u64 syscall_partition_get_next();
extern u64 syscall_partition_table_descriptor_get_data();
extern u64 syscall_partition_table_descriptor_get_next();
extern u64 syscall_pipe_create();
extern u64 syscall_process_get_event();
extern u64 syscall_process_get_main_thread();
extern u64 syscall_process_get_parent();
extern u64 syscall_process_get_pid();
extern u64 syscall_process_set_cwd();
extern u64 syscall_process_set_root();
extern u64 syscall_process_start();
extern u64 syscall_scheduler_yield();
extern u64 syscall_signature_verify();
extern u64 syscall_socket_bind();
extern u64 syscall_socket_connect();
extern u64 syscall_socket_create();
extern u64 syscall_socket_create_pair();
extern u64 syscall_socket_recv();
extern u64 syscall_socket_send();
extern u64 syscall_socket_shutdown();
extern u64 syscall_syscall_table_get_offset();
extern u64 syscall_system_shutdown();
extern u64 syscall_thread_await_events();
extern u64 syscall_thread_create();
extern u64 syscall_thread_get_priority();
extern u64 syscall_thread_get_tid();
extern u64 syscall_thread_set_priority();
extern u64 syscall_thread_start();
extern u64 syscall_thread_stop();
extern u64 syscall_time_get_boot_offset();
extern u64 syscall_timer_create();
extern u64 syscall_timer_delete();
extern u64 syscall_timer_get_deadline();
extern u64 syscall_timer_get_event();
extern u64 syscall_timer_update();
extern u64 syscall_uid_get();
extern u64 syscall_uid_get_name();
extern u64 syscall_uid_set();



const syscall_callback_t _syscall_kernel_functions[]={
	[1]=syscall_clock_get_converion,
	[2]=syscall_cpu_get_count,
	[3]=syscall_fd_close,
	[4]=syscall_fd_dup,
	[5]=syscall_fd_iter_get,
	[6]=syscall_fd_iter_next,
	[7]=syscall_fd_iter_start,
	[8]=syscall_fd_iter_stop,
	[9]=syscall_fd_open,
	[10]=syscall_fd_path,
	[11]=syscall_fd_read,
	[12]=syscall_fd_resize,
	[13]=syscall_fd_seek,
	[14]=syscall_fd_stat,
	[15]=syscall_fd_write,
	[16]=syscall_memory_map,
	[17]=syscall_memory_change_flags,
	[18]=syscall_memory_unmap,
	[19]=syscall_system_shutdown,
	[20]=syscall_thread_create,
	[22]=syscall_thread_get_priority,
	[24]=syscall_thread_set_priority,
	[25]=syscall_thread_stop,
	[26]=syscall_time_get_boot_offset,
	[27]=syscall_uid_get,
	[28]=syscall_gid_get,
	[29]=syscall_uid_set,
	[30]=syscall_gid_set,
	[31]=syscall_uid_get_name,
	[32]=syscall_gid_get_name,
	[33]=syscall_process_get_pid,
	[34]=syscall_thread_get_tid,
	[35]=syscall_process_start,
	[36]=syscall_thread_await_events,
	[37]=syscall_process_get_event,
	[38]=syscall_syscall_table_get_offset,
	[39]=syscall_scheduler_yield,
	[40]=syscall_acl_get_permissions,
	[41]=syscall_acl_set_permissions,
	[42]=syscall_acl_request_permissions,
	[43]=syscall_socket_create,
	[44]=syscall_socket_recv,
	[45]=syscall_socket_send,
	[46]=syscall_socket_shutdown,
	[47]=syscall_socket_bind,
	[48]=syscall_socket_connect,
	[49]=syscall_socket_create_pair,
	[50]=syscall_timer_create,
	[51]=syscall_timer_delete,
	[52]=syscall_timer_get_deadline,
	[53]=syscall_timer_update,
	[54]=syscall_timer_get_event,
	[55]=syscall_event_create,
	[56]=syscall_event_delete,
	[57]=syscall_event_dispatch,
	[58]=syscall_event_set_active,
	[59]=syscall_fs_get_next,
	[60]=syscall_fs_get_data,
	[61]=syscall_fs_mount,
	[62]=syscall_process_set_cwd,
	[63]=syscall_process_get_parent,
	[64]=syscall_process_set_root,
	[65]=syscall_pipe_create,
	[66]=syscall_signature_verify,
	[67]=syscall_memory_get_size,
	[68]=syscall_fs_descriptor_get_next,
	[69]=syscall_fs_descriptor_get_data,
	[70]=syscall_partition_get_next,
	[71]=syscall_partition_get_data,
	[72]=syscall_drive_get_next,
	[73]=syscall_drive_get_data,
	[74]=syscall_partition_table_descriptor_get_next,
	[75]=syscall_partition_table_descriptor_get_data,
	[76]=syscall_process_get_main_thread,
	[77]=syscall_thread_start,
};
const u64 _syscall_kernel_count=sizeof(_syscall_kernel_functions)/sizeof(syscall_callback_t);

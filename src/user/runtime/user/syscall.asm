%macro DECL_SYSCALL 3
section .text._syscall_%1
global _syscall_%1
_syscall_%1:
	mov rax, %2
%if %3==4
	mov r8, rcx
%endif
	syscall
	ret
%endmacro



[bits 64]
DECL_SYSCALL serial_send,0,2
DECL_SYSCALL serial_recv,1,3
DECL_SYSCALL elf_load,2,2
DECL_SYSCALL cpu_core_count,3,0
DECL_SYSCALL cpu_core_start,4,4
DECL_SYSCALL cpu_core_stop,5,0
DECL_SYSCALL drive_list_length,6,0
DECL_SYSCALL drive_list_get,7,3
DECL_SYSCALL partition_count,8,0
DECL_SYSCALL partition_get,9,3
DECL_SYSCALL fd_open,10,4
DECL_SYSCALL fd_close,11,1
DECL_SYSCALL fd_delete,12,1
DECL_SYSCALL fd_read,13,3
DECL_SYSCALL fd_write,14,3
DECL_SYSCALL fd_seek,15,3
DECL_SYSCALL fd_resize,16,2
DECL_SYSCALL fd_absolute_path,17,3
DECL_SYSCALL fd_stat,18,3
DECL_SYSCALL fd_get_relative,19,3
DECL_SYSCALL fd_move,20,2
DECL_SYSCALL network_layer1_config,21,2
DECL_SYSCALL network_layer2_send,22,2
DECL_SYSCALL network_layer2_poll,23,3
DECL_SYSCALL network_layer3_refresh,24,0
DECL_SYSCALL network_layer3_device_count,25,0
DECL_SYSCALL network_layer3_device_get,26,3
DECL_SYSCALL network_layer3_device_delete,27,3
DECL_SYSCALL system_shutdown,28,1
DECL_SYSCALL system_config,29,2
DECL_SYSCALL memory_map,30,2
DECL_SYSCALL memory_unmap,31,2
DECL_SYSCALL memory_stats,32,2
DECL_SYSCALL clock_get_converion,33,0
DECL_SYSCALL drive_format,34,3
DECL_SYSCALL drive_stats,35,3

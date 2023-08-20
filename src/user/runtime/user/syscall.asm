%macro DECL_SYSCALL 3
section .text._syscall_%1
global _syscall_%1
_syscall_%1:
	mov rax, %2
%if %3>=4
	mov r8, rcx
%endif
	syscall
	ret
%endmacro



[bits 64]
DECL_SYSCALL empty, 0, 0
DECL_SYSCALL serial_send, 1, 2
DECL_SYSCALL serial_recv, 2, 3
DECL_SYSCALL elf_load, 3, 2
DECL_SYSCALL cpu_core_count, 4, 0
DECL_SYSCALL cpu_core_start, 5, 4
DECL_SYSCALL cpu_core_stop, 6, 0
DECL_SYSCALL drive_list_length, 7, 0
DECL_SYSCALL drive_list_get, 8, 3
DECL_SYSCALL partition_count, 9, 0
DECL_SYSCALL partition_get, 10, 3
DECL_SYSCALL fd_open, 11, 4
DECL_SYSCALL fd_close, 12, 1
DECL_SYSCALL fd_delete, 13, 1
DECL_SYSCALL fd_read, 14, 3
DECL_SYSCALL fd_write, 15, 3
DECL_SYSCALL fd_seek, 16, 3
DECL_SYSCALL fd_resize, 17, 2
DECL_SYSCALL fd_absolute_path, 18, 3
DECL_SYSCALL fd_stat, 19, 3
DECL_SYSCALL fd_get_relative, 20, 3
DECL_SYSCALL fd_move, 21, 2
DECL_SYSCALL network_layer1_config, 22, 2
DECL_SYSCALL network_layer2_send, 23, 2
DECL_SYSCALL network_layer2_poll, 24, 3
DECL_SYSCALL network_layer3_refresh, 25, 0
DECL_SYSCALL network_layer3_device_count, 26, 0
DECL_SYSCALL network_layer3_device_get, 27, 3
DECL_SYSCALL network_layer3_device_delete, 28, 3
DECL_SYSCALL system_shutdown, 29, 1
DECL_SYSCALL system_config, 30, 2
DECL_SYSCALL memory_map, 31, 2
DECL_SYSCALL memory_unmap, 32, 2
DECL_SYSCALL memory_stats, 33, 2
DECL_SYSCALL clock_get_converion, 34, 0
DECL_SYSCALL drive_format, 35, 3
DECL_SYSCALL drive_stats, 36, 3
DECL_SYSCALL random_generate, 37, 2
DECL_SYSCALL numa_node_count, 38, 0

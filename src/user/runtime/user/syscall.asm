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
DECL_SYSCALL fd_open, 4, 4
DECL_SYSCALL fd_close, 5, 1
DECL_SYSCALL fd_delete, 6, 1
DECL_SYSCALL fd_read, 7, 3
DECL_SYSCALL fd_write, 8, 3
DECL_SYSCALL fd_seek, 9, 3
DECL_SYSCALL fd_resize, 10, 2
DECL_SYSCALL fd_absolute_path, 11, 3
DECL_SYSCALL fd_stat, 12, 3
DECL_SYSCALL fd_get_relative, 13, 3
DECL_SYSCALL fd_move, 14, 2
DECL_SYSCALL network_layer2_send, 15, 2
DECL_SYSCALL network_layer2_poll, 16, 3
DECL_SYSCALL network_layer3_refresh, 17, 0
DECL_SYSCALL network_layer3_device_count, 18, 0
DECL_SYSCALL network_layer3_device_get, 19, 3
DECL_SYSCALL network_layer3_device_delete, 20, 3
DECL_SYSCALL system_shutdown, 21, 1
DECL_SYSCALL memory_map, 22, 2
DECL_SYSCALL memory_unmap, 23, 2
DECL_SYSCALL memory_counter_count, 24, 0
DECL_SYSCALL memory_counter, 25, 3
DECL_SYSCALL clock_get_converion, 26, 0
DECL_SYSCALL drive_format, 27, 3
DECL_SYSCALL drive_stats, 28, 3
DECL_SYSCALL random_generate, 29, 2
DECL_SYSCALL coverage_dump_data, 30, 0
DECL_SYSCALL user_data_pointer, 31, 0
DECL_SYSCALL thread_stop, 32, 0
DECL_SYSCALL thread_create, 33, 4
DECL_SYSCALL thread_get_priority, 34, 1
DECL_SYSCALL thread_set_priority, 35, 2

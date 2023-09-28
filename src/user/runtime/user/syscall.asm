%macro DECL_SYSCALL 3
section .text._syscall_%1 exec nowrite
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
DECL_SYSCALL memory_get_counter_count, 24, 0
DECL_SYSCALL memory_get_counter, 25, 3
DECL_SYSCALL memory_get_object_counter_count, 26, 0
DECL_SYSCALL memory_get_object_counter, 27, 3
DECL_SYSCALL clock_get_converion, 28, 0
DECL_SYSCALL drive_format, 29, 3
DECL_SYSCALL drive_stats, 30, 3
DECL_SYSCALL random_generate, 31, 2
DECL_SYSCALL coverage_dump_data, 32, 0
DECL_SYSCALL user_data_pointer, 33, 0
DECL_SYSCALL thread_stop, 34, 0
DECL_SYSCALL thread_create, 35, 4
DECL_SYSCALL thread_get_priority, 36, 1
DECL_SYSCALL thread_set_priority, 37, 2
DECL_SYSCALL handle_get_type_count, 38, 0
DECL_SYSCALL handle_get_type, 39, 3
DECL_SYSCALL scheduler_get_stats, 40, 3

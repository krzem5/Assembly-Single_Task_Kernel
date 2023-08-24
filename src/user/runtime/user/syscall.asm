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
DECL_SYSCALL fd_open, 7, 4
DECL_SYSCALL fd_close, 8, 1
DECL_SYSCALL fd_delete, 9, 1
DECL_SYSCALL fd_read, 10, 3
DECL_SYSCALL fd_write, 11, 3
DECL_SYSCALL fd_seek, 12, 3
DECL_SYSCALL fd_resize, 13, 2
DECL_SYSCALL fd_absolute_path, 14, 3
DECL_SYSCALL fd_stat, 15, 3
DECL_SYSCALL fd_get_relative, 16, 3
DECL_SYSCALL fd_move, 17, 2
DECL_SYSCALL network_layer2_send, 18, 2
DECL_SYSCALL network_layer2_poll, 19, 3
DECL_SYSCALL network_layer3_refresh, 20, 0
DECL_SYSCALL network_layer3_device_count, 21, 0
DECL_SYSCALL network_layer3_device_get, 22, 3
DECL_SYSCALL network_layer3_device_delete, 23, 3
DECL_SYSCALL system_shutdown, 24, 1
DECL_SYSCALL memory_map, 25, 2
DECL_SYSCALL memory_unmap, 26, 2
DECL_SYSCALL memory_stats, 27, 2
DECL_SYSCALL clock_get_converion, 28, 0
DECL_SYSCALL drive_format, 29, 3
DECL_SYSCALL drive_stats, 30, 3
DECL_SYSCALL random_generate, 31, 2
DECL_SYSCALL coverage_dump_data, 32, 0
DECL_SYSCALL user_data_pointer, 33, 0

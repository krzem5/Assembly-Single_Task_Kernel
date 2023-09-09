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
DECL_SYSCALL cpu_core_start, 4, 4
DECL_SYSCALL cpu_core_stop, 5, 0
DECL_SYSCALL fd_open, 6, 4
DECL_SYSCALL fd_close, 7, 1
DECL_SYSCALL fd_delete, 8, 1
DECL_SYSCALL fd_read, 9, 3
DECL_SYSCALL fd_write, 10, 3
DECL_SYSCALL fd_seek, 11, 3
DECL_SYSCALL fd_resize, 12, 2
DECL_SYSCALL fd_absolute_path, 13, 3
DECL_SYSCALL fd_stat, 14, 3
DECL_SYSCALL fd_get_relative, 15, 3
DECL_SYSCALL fd_move, 16, 2
DECL_SYSCALL network_layer2_send, 17, 2
DECL_SYSCALL network_layer2_poll, 18, 3
DECL_SYSCALL network_layer3_refresh, 19, 0
DECL_SYSCALL network_layer3_device_count, 20, 0
DECL_SYSCALL network_layer3_device_get, 21, 3
DECL_SYSCALL network_layer3_device_delete, 22, 3
DECL_SYSCALL system_shutdown, 23, 1
DECL_SYSCALL memory_map, 24, 2
DECL_SYSCALL memory_unmap, 25, 2
DECL_SYSCALL memory_stats, 26, 2
DECL_SYSCALL clock_get_converion, 27, 0
DECL_SYSCALL drive_format, 28, 3
DECL_SYSCALL drive_stats, 29, 3
DECL_SYSCALL random_generate, 30, 2
DECL_SYSCALL coverage_dump_data, 31, 0
DECL_SYSCALL user_data_pointer, 32, 0

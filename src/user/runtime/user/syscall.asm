global _syscall_print_string
global _syscall_print_string
global _syscall_elf_load
global _syscall_cpu_core_count
global _syscall_cpu_core_start
global _syscall_cpu_core_stop
global _syscall_drive_list_length
global _syscall_drive_list_get
global _syscall_file_system_count
global _syscall_file_system_get
global _syscall_fd_open
global _syscall_fd_close
global _syscall_fd_delete
global _syscall_fd_read
global _syscall_fd_write
global _syscall_fd_seek
global _syscall_fd_stat
global _syscall_fd_get_relative
global _syscall_fd_dup
global _syscall_net_send
global _syscall_net_poll
global _syscall_acpi_shutdown
section .text



[bits 64]
_syscall_print_string:
	mov rax, 0
	syscall
	ret



_syscall_elf_load:
	mov rax, 1
	syscall
	ret



_syscall_cpu_core_count:
	mov rax, 2
	syscall
	ret



_syscall_cpu_core_start:
	mov rax, 3
	syscall
	ret



_syscall_cpu_core_stop:
	mov rax, 4
	syscall
	ret



_syscall_drive_list_length:
	mov rax, 5
	syscall
	ret



_syscall_drive_list_get:
	mov rax, 6
	syscall
	ret



_syscall_file_system_count:
	mov rax, 7
	syscall
	ret



_syscall_file_system_get:
	mov rax, 8
	syscall
	ret



_syscall_fd_open:
	mov rax, 9
	syscall
	ret



_syscall_fd_close:
	mov rax, 10
	syscall
	ret



_syscall_fd_delete:
	mov rax, 11
	syscall
	ret



_syscall_fd_read:
	mov rax, 12
	syscall
	ret



_syscall_fd_write:
	mov rax, 13
	syscall
	ret



_syscall_fd_seek:
	mov rax, 14
	syscall
	ret



_syscall_fd_stat:
	mov rax, 15
	syscall
	ret



_syscall_fd_get_relative:
	mov rax, 16
	syscall
	ret



_syscall_fd_dup:
	mov rax, 17
	syscall
	ret



_syscall_net_send:
	mov rax, 18
	syscall
	ret



_syscall_net_poll:
	mov rax, 19
	syscall
	ret



_syscall_acpi_shutdown:
	mov rax, 21
	syscall
	ret

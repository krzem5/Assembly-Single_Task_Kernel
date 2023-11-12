[bits 64]



section .text._syscall_invalid exec nowrite
global _syscall_invalid
_syscall_invalid:
	mov rax, 0
	syscall
	ret



section .text._syscall_clock_get_converion exec nowrite
global _syscall_clock_get_converion
_syscall_clock_get_converion:
	mov rax, 1
	syscall
	ret



section .text._syscall_cpu_get_count exec nowrite
global _syscall_cpu_get_count
_syscall_cpu_get_count:
	mov rax, 2
	syscall
	ret



section .text._syscall_fd_close exec nowrite
global _syscall_fd_close
_syscall_fd_close:
	mov rax, 3
	syscall
	ret



section .text._syscall_fd_dup exec nowrite
global _syscall_fd_dup
_syscall_fd_dup:
	mov rax, 4
	syscall
	ret



section .text._syscall_fd_iter_get exec nowrite
global _syscall_fd_iter_get
_syscall_fd_iter_get:
	mov rax, 5
	syscall
	ret



section .text._syscall_fd_iter_next exec nowrite
global _syscall_fd_iter_next
_syscall_fd_iter_next:
	mov rax, 6
	syscall
	ret



section .text._syscall_fd_iter_start exec nowrite
global _syscall_fd_iter_start
_syscall_fd_iter_start:
	mov rax, 7
	syscall
	ret



section .text._syscall_fd_iter_stop exec nowrite
global _syscall_fd_iter_stop
_syscall_fd_iter_stop:
	mov rax, 8
	syscall
	ret



section .text._syscall_fd_open exec nowrite
global _syscall_fd_open
_syscall_fd_open:
	mov rax, 9
	mov r8, rcx
	syscall
	ret



section .text._syscall_fd_path exec nowrite
global _syscall_fd_path
_syscall_fd_path:
	mov rax, 10
	syscall
	ret



section .text._syscall_fd_read exec nowrite
global _syscall_fd_read
_syscall_fd_read:
	mov rax, 11
	mov r8, rcx
	syscall
	ret



section .text._syscall_fd_resize exec nowrite
global _syscall_fd_resize
_syscall_fd_resize:
	mov rax, 12
	syscall
	ret



section .text._syscall_fd_seek exec nowrite
global _syscall_fd_seek
_syscall_fd_seek:
	mov rax, 13
	syscall
	ret



section .text._syscall_fd_stat exec nowrite
global _syscall_fd_stat
_syscall_fd_stat:
	mov rax, 14
	syscall
	ret



section .text._syscall_fd_write exec nowrite
global _syscall_fd_write
_syscall_fd_write:
	mov rax, 15
	mov r8, rcx
	syscall
	ret



section .text._syscall_memory_map exec nowrite
global _syscall_memory_map
_syscall_memory_map:
	mov rax, 16
	syscall
	ret



section .text._syscall_memory_unmap exec nowrite
global _syscall_memory_unmap
_syscall_memory_unmap:
	mov rax, 17
	syscall
	ret



section .text._syscall_system_shutdown exec nowrite
global _syscall_system_shutdown
_syscall_system_shutdown:
	mov rax, 18
	syscall
	ret



section .text._syscall_thread_create exec nowrite
global _syscall_thread_create
_syscall_thread_create:
	mov rax, 19
	mov r8, rcx
	syscall
	ret



section .text._syscall_thread_get_cpu_mask exec nowrite
global _syscall_thread_get_cpu_mask
_syscall_thread_get_cpu_mask:
	mov rax, 20
	syscall
	ret



section .text._syscall_thread_get_priority exec nowrite
global _syscall_thread_get_priority
_syscall_thread_get_priority:
	mov rax, 21
	syscall
	ret



section .text._syscall_thread_set_cpu_mask exec nowrite
global _syscall_thread_set_cpu_mask
_syscall_thread_set_cpu_mask:
	mov rax, 22
	syscall
	ret



section .text._syscall_thread_set_priority exec nowrite
global _syscall_thread_set_priority
_syscall_thread_set_priority:
	mov rax, 23
	syscall
	ret



section .text._syscall_thread_stop exec nowrite
global _syscall_thread_stop
_syscall_thread_stop:
	mov rax, 24
	syscall
	ret

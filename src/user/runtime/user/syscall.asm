global _syscall_serial_send
global _syscall_serial_recv
global _syscall_elf_load
global _syscall_cpu_core_count
global _syscall_cpu_core_start
global _syscall_cpu_core_stop
global _syscall_drive_list_length
global _syscall_drive_list_get
global _syscall_partition_count
global _syscall_partition_get
global _syscall_fd_open
global _syscall_fd_close
global _syscall_fd_delete
global _syscall_fd_read
global _syscall_fd_write
global _syscall_fd_seek
global _syscall_fd_stat
global _syscall_fd_get_relative
global _syscall_fd_move
global _syscall_network_layer1_config
global _syscall_network_layer2_send
global _syscall_network_layer2_poll
global _syscall_system_shutdown
global _syscall_system_config
global _syscall_memory_map
global _syscall_memory_unmap
global _syscall_memory_stats
global _syscall_clock_get_converion
global _syscall_drive_format
global _syscall_drive_stats
global _syscall_network_layer3_refresh
global _syscall_network_layer3_device_count
global _syscall_network_layer3_device_get
section .text



[bits 64]
_syscall_serial_send:
	mov rax, 0
	syscall
	ret



_syscall_serial_recv:
	mov rax, 1
	syscall
	ret



_syscall_elf_load:
	mov rax, 2
	syscall
	ret



_syscall_cpu_core_count:
	mov rax, 3
	syscall
	ret



_syscall_cpu_core_start:
	mov r8, rcx
	mov rax, 4
	syscall
	ret



_syscall_cpu_core_stop:
	mov rax, 5
	syscall
	ret



_syscall_drive_list_length:
	mov rax, 6
	syscall
	ret



_syscall_drive_list_get:
	mov rax, 7
	syscall
	ret



_syscall_partition_count:
	mov rax, 8
	syscall
	ret



_syscall_partition_get:
	mov rax, 9
	syscall
	ret



_syscall_fd_open:
	mov r8, rcx
	mov rax, 10
	syscall
	ret



_syscall_fd_close:
	mov rax, 11
	syscall
	ret



_syscall_fd_delete:
	mov rax, 12
	syscall
	ret



_syscall_fd_read:
	mov rax, 13
	syscall
	ret



_syscall_fd_write:
	mov rax, 14
	syscall
	ret



_syscall_fd_seek:
	mov rax, 15
	syscall
	ret



_syscall_fd_stat:
	mov rax, 16
	syscall
	ret



_syscall_fd_get_relative:
	mov rax, 17
	syscall
	ret



_syscall_fd_move:
	mov rax, 18
	syscall
	ret



_syscall_network_layer1_config:
	mov rax, 19
	syscall
	ret



_syscall_network_layer2_send:
	mov rax, 20
	syscall
	ret



_syscall_network_layer2_poll:
	mov rax, 21
	syscall
	ret



_syscall_system_shutdown:
	mov rax, 22
	syscall
	ret



_syscall_system_config:
	mov rax, 23
	syscall
	ret



_syscall_memory_map:
	mov rax, 24
	syscall
	ret



_syscall_memory_unmap:
	mov rax, 25
	syscall
	ret



_syscall_memory_stats:
	mov rax, 26
	syscall
	ret



_syscall_clock_get_converion:
	mov rax, 27
	syscall
	ret



_syscall_drive_format:
	mov rax, 28
	syscall
	ret



_syscall_drive_stats:
	mov rax, 29
	syscall
	ret



_syscall_network_layer3_refresh:
	mov rax, 30
	syscall
	ret



_syscall_network_layer3_device_count:
	mov rax, 31
	syscall
	ret



_syscall_network_layer3_device_get:
	mov rax, 32
	syscall
	ret

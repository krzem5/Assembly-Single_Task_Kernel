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



section .text._syscall_coverage_dump_data exec nowrite
global _syscall_coverage_dump_data
_syscall_coverage_dump_data:
	mov rax, 2
	syscall
	ret



section .text._syscall_drive_format exec nowrite
global _syscall_drive_format
_syscall_drive_format:
	mov rax, 3
	syscall
	ret



section .text._syscall_drive_stats exec nowrite
global _syscall_drive_stats
_syscall_drive_stats:
	mov rax, 4
	syscall
	ret



section .text._syscall_elf_load exec nowrite
global _syscall_elf_load
_syscall_elf_load:
	mov rax, 5
	syscall
	ret



section .text._syscall_fd_absolute_path exec nowrite
global _syscall_fd_absolute_path
_syscall_fd_absolute_path:
	mov rax, 6
	syscall
	ret



section .text._syscall_fd_close exec nowrite
global _syscall_fd_close
_syscall_fd_close:
	mov rax, 7
	syscall
	ret



section .text._syscall_fd_delete exec nowrite
global _syscall_fd_delete
_syscall_fd_delete:
	mov rax, 8
	syscall
	ret



section .text._syscall_fd_get_relative exec nowrite
global _syscall_fd_get_relative
_syscall_fd_get_relative:
	mov rax, 9
	syscall
	ret



section .text._syscall_fd_move exec nowrite
global _syscall_fd_move
_syscall_fd_move:
	mov rax, 10
	syscall
	ret



section .text._syscall_fd_open exec nowrite
global _syscall_fd_open
_syscall_fd_open:
	mov rax, 11
	mov r8, rcx
	syscall
	ret



section .text._syscall_fd_read exec nowrite
global _syscall_fd_read
_syscall_fd_read:
	mov rax, 12
	syscall
	ret



section .text._syscall_fd_resize exec nowrite
global _syscall_fd_resize
_syscall_fd_resize:
	mov rax, 13
	syscall
	ret



section .text._syscall_fd_seek exec nowrite
global _syscall_fd_seek
_syscall_fd_seek:
	mov rax, 14
	syscall
	ret



section .text._syscall_fd_stat exec nowrite
global _syscall_fd_stat
_syscall_fd_stat:
	mov rax, 15
	syscall
	ret



section .text._syscall_fd_write exec nowrite
global _syscall_fd_write
_syscall_fd_write:
	mov rax, 16
	syscall
	ret



section .text._syscall_handle_get_type exec nowrite
global _syscall_handle_get_type
_syscall_handle_get_type:
	mov rax, 17
	syscall
	ret



section .text._syscall_handle_get_type_count exec nowrite
global _syscall_handle_get_type_count
_syscall_handle_get_type_count:
	mov rax, 18
	syscall
	ret



section .text._syscall_memory_get_counter exec nowrite
global _syscall_memory_get_counter
_syscall_memory_get_counter:
	mov rax, 19
	syscall
	ret



section .text._syscall_memory_get_counter_count exec nowrite
global _syscall_memory_get_counter_count
_syscall_memory_get_counter_count:
	mov rax, 20
	syscall
	ret



section .text._syscall_memory_get_object_counter exec nowrite
global _syscall_memory_get_object_counter
_syscall_memory_get_object_counter:
	mov rax, 21
	syscall
	ret



section .text._syscall_memory_get_object_counter_count exec nowrite
global _syscall_memory_get_object_counter_count
_syscall_memory_get_object_counter_count:
	mov rax, 22
	syscall
	ret



section .text._syscall_memory_map exec nowrite
global _syscall_memory_map
_syscall_memory_map:
	mov rax, 23
	syscall
	ret



section .text._syscall_memory_unmap exec nowrite
global _syscall_memory_unmap
_syscall_memory_unmap:
	mov rax, 24
	syscall
	ret



section .text._syscall_network_layer2_poll exec nowrite
global _syscall_network_layer2_poll
_syscall_network_layer2_poll:
	mov rax, 25
	syscall
	ret



section .text._syscall_network_layer2_send exec nowrite
global _syscall_network_layer2_send
_syscall_network_layer2_send:
	mov rax, 26
	syscall
	ret



section .text._syscall_network_layer3_device_count exec nowrite
global _syscall_network_layer3_device_count
_syscall_network_layer3_device_count:
	mov rax, 27
	syscall
	ret



section .text._syscall_network_layer3_device_delete exec nowrite
global _syscall_network_layer3_device_delete
_syscall_network_layer3_device_delete:
	mov rax, 28
	syscall
	ret



section .text._syscall_network_layer3_device_get exec nowrite
global _syscall_network_layer3_device_get
_syscall_network_layer3_device_get:
	mov rax, 29
	syscall
	ret



section .text._syscall_network_layer3_refresh exec nowrite
global _syscall_network_layer3_refresh
_syscall_network_layer3_refresh:
	mov rax, 30
	syscall
	ret



section .text._syscall_random_generate exec nowrite
global _syscall_random_generate
_syscall_random_generate:
	mov rax, 31
	syscall
	ret



section .text._syscall_scheduler_get_stats exec nowrite
global _syscall_scheduler_get_stats
_syscall_scheduler_get_stats:
	mov rax, 32
	syscall
	ret



section .text._syscall_scheduler_get_timers exec nowrite
global _syscall_scheduler_get_timers
_syscall_scheduler_get_timers:
	mov rax, 33
	syscall
	ret



section .text._syscall_serial_recv exec nowrite
global _syscall_serial_recv
_syscall_serial_recv:
	mov rax, 34
	syscall
	ret



section .text._syscall_serial_send exec nowrite
global _syscall_serial_send
_syscall_serial_send:
	mov rax, 35
	syscall
	ret



section .text._syscall_system_shutdown exec nowrite
global _syscall_system_shutdown
_syscall_system_shutdown:
	mov rax, 36
	syscall
	ret



section .text._syscall_thread_create exec nowrite
global _syscall_thread_create
_syscall_thread_create:
	mov rax, 37
	mov r8, rcx
	syscall
	ret



section .text._syscall_thread_get_cpu_mask exec nowrite
global _syscall_thread_get_cpu_mask
_syscall_thread_get_cpu_mask:
	mov rax, 38
	syscall
	ret



section .text._syscall_thread_get_priority exec nowrite
global _syscall_thread_get_priority
_syscall_thread_get_priority:
	mov rax, 39
	syscall
	ret



section .text._syscall_thread_set_cpu_mask exec nowrite
global _syscall_thread_set_cpu_mask
_syscall_thread_set_cpu_mask:
	mov rax, 40
	syscall
	ret



section .text._syscall_thread_set_priority exec nowrite
global _syscall_thread_set_priority
_syscall_thread_set_priority:
	mov rax, 41
	syscall
	ret



section .text._syscall_thread_stop exec nowrite
global _syscall_thread_stop
_syscall_thread_stop:
	mov rax, 42
	syscall
	ret



section .text._syscall_user_data_pointer exec nowrite
global _syscall_user_data_pointer
_syscall_user_data_pointer:
	mov rax, 43
	syscall
	ret

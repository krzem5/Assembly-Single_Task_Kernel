extern _syscall_clock_get_converion
global clock_cpu_frequency
global clock_init
global clock_get_ticks
global clock_get_time
global clock_ticks_to_time



[bits 64]
section .text.clock_init exec nowrite
clock_init:
	call _syscall_clock_get_converion
	mov qword [_clock_conversion_factor], rax
	mov dword [_clock_conversion_shift], edx
	mov qword [clock_cpu_frequency], r8
	ret



section .text.clock_get_ticks exec nowrite
clock_get_ticks:
	rdtsc
	shl rdx, 32
	or rax, rdx
	ret



section .text.clock_get_time exec nowrite
clock_get_time:
	rdtsc
	shl rdx, 32
	or rdx, rax
	mulx rdx, rax, qword [_clock_conversion_factor]
	mov ecx, dword [_clock_conversion_shift]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret



section .text.clock_ticks_to_time exec nowrite
clock_ticks_to_time:
	mov rdx, rdi
	mulx rdx, rax, qword [_clock_conversion_factor]
	mov ecx, dword [_clock_conversion_shift]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret



section .data.clock_data noexec write



align 8
clock_cpu_frequency:
	dq 0
_clock_conversion_factor:
	dq 0
_clock_conversion_shift:
	dd 0

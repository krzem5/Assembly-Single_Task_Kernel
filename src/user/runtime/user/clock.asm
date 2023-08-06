extern _syscall_clock_get_converion
global clock_cpu_frequency
global _clock_init
global clock_get_ticks
global clock_get_time
global clock_ticks_to_time
section .text



[bits 64]
_clock_init:
	call _syscall_clock_get_converion
	mov qword [_clock_conversion_factor], rax
	mov dword [_clock_conversion_shift], edx
	mov qword [clock_cpu_frequency], r8
	ret



clock_get_ticks:
	rdtsc
	shl rdx, 32
	or rax, rdx
	ret



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



clock_ticks_to_time:
	mov rdx, rdi
	mulx rdx, rax, qword [_clock_conversion_factor]
	mov ecx, dword [_clock_conversion_shift]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret



section .data



align 8
clock_cpu_frequency:
	dq 8
_clock_conversion_factor:
	dq 0
_clock_conversion_shift:
	dd 0

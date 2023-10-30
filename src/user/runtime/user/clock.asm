extern _syscall_cspinlock_get_converion
global cspinlock_cpu_frequency
global cspinlock_init
global cspinlock_get_ticks
global cspinlock_get_time
global cspinlock_ticks_to_time



[bits 64]
section .text.cspinlock_init exec nowrite
cspinlock_init:
	call _syscall_cspinlock_get_converion
	mov qword [_cspinlock_conversion_factor], rax
	mov dword [_cspinlock_conversion_shift], edx
	mov qword [cspinlock_cpu_frequency], r8
	ret



section .text.cspinlock_get_ticks exec nowrite
cspinlock_get_ticks:
	rdtsc
	shl rdx, 32
	or rax, rdx
	ret



section .text.cspinlock_get_time exec nowrite
cspinlock_get_time:
	rdtsc
	shl rdx, 32
	or rdx, rax
	mulx rdx, rax, qword [_cspinlock_conversion_factor]
	mov ecx, dword [_cspinlock_conversion_shift]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret



section .text.cspinlock_ticks_to_time exec nowrite
cspinlock_ticks_to_time:
	mov rdx, rdi
	mulx rdx, rax, qword [_cspinlock_conversion_factor]
	mov ecx, dword [_cspinlock_conversion_shift]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret



section .data.cspinlock_data noexec write



align 8
cspinlock_cpu_frequency:
	dq 0
_cspinlock_conversion_factor:
	dq 0
_cspinlock_conversion_shift:
	dd 0

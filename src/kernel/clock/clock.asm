extern cspinlock_conversion_factor
extern cspinlock_conversion_shift
global cspinlock_get_ticks
global cspinlock_get_time
global cspinlock_ticks_to_time
section .text exec nowrite



[bits 64]
cspinlock_get_ticks:
	rdtsc
	shl rdx, 32
	or rax, rdx
	ret



cspinlock_get_time:
	rdtsc
	shl rdx, 32
	or rdx, rax
	jmp _cspinlock_ticks_to_time_inernal



cspinlock_ticks_to_time:
	mov rdx, rdi
_cspinlock_ticks_to_time_inernal:
	mulx rdx, rax, qword [cspinlock_conversion_factor]
	mov ecx, dword [cspinlock_conversion_shift]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret

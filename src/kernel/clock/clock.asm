extern clock_conversion_factor
extern clock_conversion_shift
global clock_get_ticks:function default
global clock_get_time:function default
global clock_ticks_to_time:function default
section .text exec nowrite



[bits 64]
clock_get_ticks:
	rdtsc
	shl rdx, 32
	or rax, rdx
	ret



clock_get_time:
	rdtsc
	shl rdx, 32
	or rdx, rax
	jmp _clock_ticks_to_time_inernal



clock_ticks_to_time:
	mov rdx, rdi
_clock_ticks_to_time_inernal:
	mulx rdx, rax, qword [clock_conversion_factor]
	mov ecx, dword [clock_conversion_shift]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret

extern clock_conversion_factor
extern clock_conversion_shift
global clock_get_ticks
global clock_get_time
section .text



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
	mulx rdx, rax, qword [clock_conversion_factor]
	mov ecx, dword [clock_conversion_shift]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret

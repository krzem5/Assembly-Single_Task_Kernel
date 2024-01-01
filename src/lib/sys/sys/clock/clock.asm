%include "sys/types.inc"
extern _sys_clock_conversion_factor
extern _sys_clock_conversion_shift
global sys_clock_convert_ticks_to_time_ns
section .text exec nowrite



[bits 64]
sys_clock_convert_ticks_to_time_ns:
	mov rdx, rdi
	mulx rdx, rax, qword [REF_DATA(_sys_clock_conversion_factor)]
	mov ecx, dword [REF_DATA(_sys_clock_conversion_shift)]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret

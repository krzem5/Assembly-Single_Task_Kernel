%include "sys2/types.inc"
extern _sys2_clock_conversion_factor
extern _sys2_clock_conversion_shift
global sys2_clock_convert_ticks_to_time_ns
section .text exec nowrite



[bits 64]
sys2_clock_convert_ticks_to_time_ns:
	mov rdx, rdi
	mulx rdx, rax, qword [REF_DATA(_sys2_clock_conversion_factor)]
	mov ecx, dword [REF_DATA(_sys2_clock_conversion_shift)]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret

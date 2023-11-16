%include "sys/types.inc"
extern _syscall_clock_get_converion
global sys_clock_init:function _sys_clock_init_size
global sys_clock_get_ticks:function _sys_clock_get_ticks_size
global sys_clock_get_time:function _sys_clock_get_time_size
global sys_clock_ticks_to_time:function _sys_clock_ticks_to_time_size
global sys_clock_get_frequency:function _sys_clock_get_frequency_size



[bits 64]
section .text.sys_clock_init exec nowrite
sys_clock_init:
	call [REF(_syscall_clock_get_converion)]
	mov qword [REF_DATA(_sys_clock_conversion_factor)], rax
	mov dword [REF_DATA(_sys_clock_conversion_shift)], edx
	mov qword [REF_DATA(_sys_clock_cpu_frequency)], r8
	ret
_sys_clock_init_size equ $-$$



section .text.sys_clock_get_ticks exec nowrite
sys_clock_get_ticks:
	rdtsc
	shl rdx, 32
	or rax, rdx
	ret
_sys_clock_get_ticks_size equ $-$$



section .text.sys_clock_get_time exec nowrite
sys_clock_get_time:
	rdtsc
	shl rdx, 32
	or rdx, rax
	mulx rdx, rax, qword [REF_DATA(_sys_clock_conversion_factor)]
	mov ecx, dword [REF_DATA(_sys_clock_conversion_shift)]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret
_sys_clock_get_time_size equ $-$$



section .text.sys_clock_ticks_to_time exec nowrite
sys_clock_ticks_to_time:
	mov rdx, rdi
	mulx rdx, rax, qword [REF_DATA(_sys_clock_conversion_factor)]
	mov ecx, dword [REF_DATA(_sys_clock_conversion_shift)]
	shrd rax, rdx, cl
	shrx rdx, rdx, rcx
	test cl, 64
	cmovne rax, rdx
	ret
_sys_clock_ticks_to_time_size equ $-$$



section .text.sys_clock_get_frequency exec nowrite
sys_clock_get_frequency:
	mov rax, qword [REF_DATA(_sys_clock_cpu_frequency)]
	ret
_sys_clock_get_frequency_size equ $-$$



section .data.sys_clock_data noexec write



align 8
_sys_clock_cpu_frequency:
	dq 0
_sys_clock_conversion_factor:
	dq 0
_sys_clock_conversion_shift:
	dd 0

extern main
extern _syscall_cpu_core_stop
global _start
section .text



[bits 64]
_start:
	call main
	jmp _syscall_cpu_core_stop

extern _syscall__report_coverage
global main
section .text



[bits 64]
main:
	jmp _syscall__report_coverage

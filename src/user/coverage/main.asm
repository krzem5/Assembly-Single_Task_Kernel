extern _syscall_report_coverage
global main
section .text



[bits 64]
main:
	jmp _syscall_report_coverage

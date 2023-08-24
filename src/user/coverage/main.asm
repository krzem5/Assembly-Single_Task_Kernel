extern _syscall_coverage_dump_data
global main
section .text



[bits 64]
main:
	jmp _syscall_coverage_dump_data

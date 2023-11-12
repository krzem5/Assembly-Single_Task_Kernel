extern main
global _start
section .text exec nowrite



[bits 64]
[default rel]
_start:
	sub rsp, 8
	and rsp, 0xfffffffffffffff0
	mov rbp, rsp
	mov rdi, r15
	call [rel main wrt ..got]
	jmp rax

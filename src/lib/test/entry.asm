%include "core/types.inc"
extern main
global _start
section .text exec nowrite



[bits 64]
_start:
	sub rsp, 8
	and rsp, 0xfffffffffffffff0
	mov rbp, rsp
	mov rdi, r15
	jmp [REF(main)]

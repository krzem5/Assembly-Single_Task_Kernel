%include "sys/types.inc"
extern main
global _start:function hidden
section .text exec nowrite



[bits 64]
_start:
	mov rdi, r15
	call [REF(main)]
	jmp rax

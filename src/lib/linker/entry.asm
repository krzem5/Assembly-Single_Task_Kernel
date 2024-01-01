%include "sys2/types.inc"
extern __sys2_init
extern main
global _start:function hidden
section .text exec nowrite



[bits 64]
_start:
	call __sys2_init
	mov rdi, r15
	call [REF(main)]
	jmp rax

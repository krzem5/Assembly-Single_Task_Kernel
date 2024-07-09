%include "sys/types.inc"
extern __sys_init
extern main
global _start:function hidden
section .text exec nowrite



[bits 64]
_start:
	call __sys_init
	mov rdi, rsp
	call [REF(main)]
	jmp rax

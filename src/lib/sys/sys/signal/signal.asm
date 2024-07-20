%include "sys/types.inc"
global _sys_signal_handler_wrapper:function
section .text exec nowrite



[bits 64]
_sys_signal_handler_wrapper:
	push rdi
	push rsi
	push rdx
	push r10
	mov rdi, rsp
	call r8
	mov rax, 0x100000073
	add rsp, 8
	pop rdx
	pop rsi
	pop rdi
	syscall
	jmp $

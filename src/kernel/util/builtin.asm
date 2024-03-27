extern mem_copy
extern mem_fill
global memcpy:function
global memset:function
section .text exec nowrite



[bits 64]
memcpy:
	push rdi
	call mem_copy
	pop rax
	ret



memset:
	push rdi
	call mem_fill
	pop rax
	ret

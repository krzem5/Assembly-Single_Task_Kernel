%include "sys2/types.inc"
extern symbol_resolve_plt
global symbol_resolve_plt_trampoline:function hidden
section .text exec nowrite



[bits 64]
symbol_resolve_plt_trampoline:
	push rax
	push rcx
	push rdx
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	mov rdi, qword [rsp+0x48]
	mov rsi, qword [rsp+0x50]
	call [REF(symbol_resolve_plt)]
	test rax, rax
	mov qword [rsp+0x50], rax
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rax
	add rsp, 8
	jz ._symbol_not_found
	ret
._symbol_not_found:
	add rsp, 8
	ret

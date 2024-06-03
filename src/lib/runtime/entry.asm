%include "sys/types.inc"
extern _execute_fini
extern main
global _start:function hidden
section .text exec nowrite



[bits 64]
_start:
	; argc    = rsp[0]
	; argv    = rsp+1
	; environ = rsp+1+argc
	; auxv    = rsp+1+argc+#environ+1
	mov edi, dword [rsp]
	lea rsi, [rsp+8]
	lea rdx, [rsi+rdi*8]
	mov rcx, rdx
	cmp qword [rcx], 0
	je ._empty_environ
._skip_entry:
	add rcx, 8
	cmp qword [rcx], 0
	jne ._skip_entry
._empty_environ:
	add rcx, 8
	call [REF(main)]
	mov rax, rdi
	jmp _execute_fini

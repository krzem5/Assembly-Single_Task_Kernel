%include "sys/types.inc"
extern _execute_fini
extern main
global _start:function hidden
section .text exec nowrite



[bits 64]
_start:
	; argc    = r15[0]
	; argv    = r15+1
	; environ = r15+1+argc
	; auxv    = r15+1+argc+#environ+1
	mov edi, dword [r15]
	lea rsi, qword [r15+8]
	lea rdx, qword [rsi+rdi*8]
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
	call _execute_fini
	mov rax, 0x100000019
	xor edi, edi
	syscall
	jmp $

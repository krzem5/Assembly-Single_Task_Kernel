extern efi_main
global _start
section .text



[bits 64]
_start:
	sub rsp, 8
	mov rdi, rcx
	mov rsi, rdx
	call efi_main
	add rsp, 8
	ret



section .data
_relocatable_symbol:
	dq 0



section .reloc
	dd _relocatable_symbol-$
	dd 10
	dw 0

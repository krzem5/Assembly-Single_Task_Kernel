extern efi_main
global _start
section .text exec nowrite



[bits 64]
_start:
	sub rsp, 8
	mov rdi, rcx
	mov rsi, rdx
	call efi_main
	add rsp, 8
	ret



section .data noexec write
_relocatable_symbol:
	dq 0



section .reloc noexec nowrite
	dd _relocatable_symbol-$
	dd 10
	dw 0

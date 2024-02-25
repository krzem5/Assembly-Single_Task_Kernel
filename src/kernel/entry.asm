extern main
global _start:function hidden
section .entry exec nowrite



[bits 64]
_start:
	cli
	mov rsp, rdx
	or rsp, 0xffffffffc0000000
	xor rbp, rbp
	mov cr3, rsi
	mov rax, main
	jmp rax

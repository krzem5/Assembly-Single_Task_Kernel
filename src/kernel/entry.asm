extern main
global _start:function hidden
section .entry exec nowrite



[bits 64]
_start:
	cli
	mov rax, 0x1ffff
	shl rax, 47
	or rdx, rax
	mov rsp, rdx
	xor rbp, rbp
	mov cr3, rsi
	mov rax, main
	jmp rax

%define KERNEL_OFFSET 0xffffffffc0000000



extern main
global _start
section .entry exec nowrite



[bits 64]
_start:
	cli
	mov rsp, rdx
	add rsp, KERNEL_OFFSET
	xor rbp, rbp
	mov cr3, rsi
	mov al, 0x00
	mov dx, 0x3f9
	out dx, al
	mov al, 0x80
	mov dx, 0x3fb
	out dx, al
	mov al, 0x03
	mov dx, 0x3f8
	out dx, al
	mov al, 0x00
	mov dx, 0x3f9
	out dx, al
	mov al, 0x03
	mov dx, 0x3fb
	out dx, al
	mov al, 0xc7
	mov dx, 0x3fa
	out dx, al
	mov al, 0x03
	mov dx, 0x3fc
	out dx, al
	jmp (KERNEL_OFFSET+main)

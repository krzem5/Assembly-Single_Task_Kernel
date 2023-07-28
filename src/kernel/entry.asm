global _start
extern main
section .entry



[bits 64]
_start:
	;;; Initialize GS base (required for lock_acquire)
	lea rdx, cpu_data
	mov eax, edx
	mov ecx, 0xc0000101
	shr rdx, 32
	wrmsr
	;;; Start the kernel
	jmp main



section .rdata



align 8
cpu_data:
	db 0x00

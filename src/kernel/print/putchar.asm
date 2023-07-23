global _putchar_nolock
section .text



[bits 64]
_putchar_nolock:
	mov dx, 0x3fd
._poll_serial:
	in al, dx
	and al, 0x20
	jz ._poll_serial
	sub dx, 5
	mov al, dil
	out dx, al
	ret

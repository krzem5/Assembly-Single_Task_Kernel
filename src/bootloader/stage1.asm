%define KFS_HEADER_SIZE 4096



[org 0x7c00]



[bits 16]
	mov ax, (0x0200+(__BOOTLOADER_STAGE2_SIZE__+511)/512)
	mov bx, 0x0500
	mov cx, (0x0002+(KFS_HEADER_SIZE+511)/512)
	mov dh, 0x00
	int 0x13
	jmp 0x500



times 510-($-$$) db 0
dw 0xaa55
times KFS_HEADER_SIZE db 0

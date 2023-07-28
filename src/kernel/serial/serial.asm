extern lock_acquire
extern lock_release
global serial_send
global serial_recv
section .ctext



[bits 64]
serial_send:
	test esi, esi
	jz ._end
	push rdi
	lea rdi, _serial_lock
	call lock_acquire
	pop rdi
	xor ecx, ecx
	mov edx, 0x3f8
._next_char:
	add dx, 5
._poll_serial:
	in al, dx
	and al, 0x20
	jz ._poll_serial
	sub dx, 5
	mov al, byte [rdi+rcx]
	out dx, al
	add ecx, 1
	cmp ecx, esi
	jl ._next_char
	lea rdi, _serial_lock
	jmp lock_release
._end:
	ret



serial_recv:
	mov r8, rdx
	push rdi
	lea rdi, _serial_lock
	call lock_acquire
	pop rdi
	xor ecx, ecx
	test esi, esi
	jz ._end
	mov dx, 0x3f8
._next_char:
	add dx, 5
	mov r9, r8
._poll_serial:
	sub r9, 1
	jz ._end
	in al, dx
	and al, 0x01
	jz ._poll_serial
	sub dx, 5
	in al, dx
	mov byte [rdi+rcx], al
	add ecx, 1
	cmp ecx, esi
	jl ._next_char
._end:
	lea rdi, _serial_lock
	call lock_release
	mov eax, ecx
	ret



section .cdata



align 4
_serial_lock:
	dd 0

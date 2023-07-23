global lock_init
global lock_acquire
global lock_release
section .text



[bits 64]
lock_acquire:
	xor eax, eax
	movzx edx, byte [gs:0]
	add edx, 1
._retry:
	lock cmpxchg dword [rdi], edx
	or eax, 0
	jnz ._retry
	ret



lock_init:
lock_release:
	xor eax, eax
	lock xchg dword [rdi], eax
	ret

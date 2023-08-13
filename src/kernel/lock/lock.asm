global lock_init
global lock_acquire
global lock_release
global lock_acquire_multiple
global lock_release_multiple
section .ctext



[bits 64]
lock_acquire:
	xor eax, eax
	movzx edx, byte [gs:0]
	add edx, 1
._retry:
	lock cmpxchg dword [rdi], edx
	test eax, eax
	jnz ._retry
	ret



lock_init:
lock_release:
	xor eax, eax
	lock xchg dword [rdi], eax
	ret



lock_acquire_multiple:
	xor eax, eax
	mov edx, 0x80000001
._retry:
	lock cmpxchg dword [rdi], edx
	bt eax, 31
	jc ._increase
	test eax, eax
	jnz ._retry
	ret
._increase:
	lock bts dword [rdi], 31
	lock add dword [rdi], 1
	ret



lock_release_multiple:
	mov eax, 0x80000001
	mov edx, 1
	lock cmpxchg dword [rdi], edx
	lock sub dword [rdi], 1
	ret

global lock_init
global lock_acquire
global lock_release
global lock_acquire_multiple
global lock_release_multiple
section .ctext



[bits 64]
_lock_acquire_wait:
	pause
	cmp dword [rdi], 0
	jnz _lock_acquire_wait
lock_acquire:
	lock bts dword [rdi], 0
	jc _lock_acquire_wait
	ret



lock_init:
lock_release:
	mov dword [rdi], 0
	ret



lock_acquire_multiple:
	xor ecx, ecx
._retry:
	mov eax, dword [rdi]
	lea edx, [eax+1]
	bt eax, 31
	cmovnc eax, ecx
	lock cmpxchg dword [rdi], edx
	jz ._retry
	ret



lock_release_multiple:
	mov eax, 0x80000001
	mov edx, 1
	lock cmpxchg dword [rdi], edx
	lock sub dword [rdi], 1
	ret

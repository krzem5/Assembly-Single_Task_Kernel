;;; bit 0 - global lock, bit 1 - multiaccess lock, bit 2 - multiaccess active, bit 3-31 multiaccess counter



global lock_init
global lock_acquire
global lock_release
global lock_acquire_multiple
global lock_release_multiple



[bits 64]
section .text.lock_acquire
_lock_acquire_wait:
	pause
	test dword [rdi], 1
	jnz _lock_acquire_wait
lock_acquire:
	lock bts dword [rdi], 0
	jc _lock_acquire_wait
	ret



section .text.lock_release
lock_init:
lock_release:
	btr dword [rdi], 0
	ret



section .text.lock_acquire_multiple
_lock_acquire_multiple_wait:
	pause
	test dword [rdi], 2
	jnz _lock_acquire_multiple_wait
lock_acquire_multiple:
	lock bts dword [rdi], 1
	jc _lock_acquire_multiple_wait
	test dword [rdi], 4
	jnz ._multiaccess_active
	jmp ._global_lock_test
._global_lock_wait:
	pause
	test dword [rdi], 1
	jnz ._global_lock_wait
._global_lock_test:
	lock bts dword [rdi], 0
	jc ._global_lock_wait
	bts dword [rdi], 2
._multiaccess_active:
	add dword [rdi], 8
	btr dword [rdi], 1
	ret



section .text.lock_release_multiple
_lock_release_multiple_wait:
	pause
	test dword [rdi], 2
	jnz _lock_release_multiple_wait
lock_release_multiple:
	lock bts dword [rdi], 1
	jc _lock_release_multiple_wait
	sub dword [rdi], 8
	cmp dword [rdi], 8
	jge ._still_used
	mov dword [rdi], 0
	ret
._still_used:
	btr dword [rdi], 1
	ret

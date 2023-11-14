global lock_init:function _lock_init_size
global lock_acquire_exclusive:function _lock_acquire_exclusive_size
global lock_release_exclusive:function _lock_release_exclusive_size
global lock_acquire_shared:function _lock_acquire_shared_size
global lock_release_shared:function _lock_release_shared_size



[bits 64]
section .text.lock_init exec nowrite
lock_init:
	mov dword [rdi], 0
	ret
_lock_init_size equ $-$$



section .text.lock_acquire_exclusive exec nowrite
_lock_acquire_exclusive_global_wait:
	pause
	test dword [rdi], 1
	jnz _lock_acquire_exclusive_global_wait
lock_acquire_exclusive:
	lock bts dword [rdi], 0
	jc _lock_acquire_exclusive_global_wait
	ret
_lock_acquire_exclusive_size equ $-$$



section .text.lock_release_exclusive exec nowrite
lock_release_exclusive:
	btr dword [rdi], 0
	ret
_lock_release_exclusive_size equ $-$$



section .text.lock_acquire_shared exec nowrite
_lock_acquire_shared_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz _lock_acquire_shared_multiaccess_wait
lock_acquire_shared:
	lock bts dword [rdi], 1
	jc _lock_acquire_shared_multiaccess_wait
	test dword [rdi], 4
	jnz ._multiaccess_active
	jmp ._global_test
._global_wait:
	pause
	test dword [rdi], 1
	jnz ._global_wait
._global_test:
	lock bts dword [rdi], 0
	jc ._global_wait
	bts dword [rdi], 2
._multiaccess_active:
	add dword [rdi], 8
	btr dword [rdi], 1
	ret
_lock_acquire_shared_size equ $-$$



section .text.lock_release_shared exec nowrite
_lock_release_shared_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz _lock_release_shared_multiaccess_wait
lock_release_shared:
	lock bts dword [rdi], 1
	jc _lock_release_shared_multiaccess_wait
	sub dword [rdi], 8
	cmp dword [rdi], 8
	jge ._still_used
	mov dword [rdi], 0
	ret
._still_used:
	btr dword [rdi], 1
	ret
_lock_release_shared_size equ $-$$

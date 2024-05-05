;;; bit 0 - global lock
;;; bit 1 - multiaccess lock
;;; bit 2 - multiaccess active
;;; bits 3-31 - multiaccess counter



global __rwlock_init:function default
global __rwlock_acquire_write:function default
global __rwlock_release_write:function default
global __rwlock_acquire_read:function default
global __rwlock_release_read:function default
global rwlock_is_held:function default
extern scheduler_pause
extern scheduler_resume
section .text exec nowrite



[bits 64]
__rwlock_init:
	mov dword [rdi], 0
	ret



__rwlock_acquire_write:
	push rdi
	call scheduler_pause
	pop rdi
	lock bts dword [rdi], 0
	jc ___rwlock_acquire_write_global_wait
	ret
___rwlock_acquire_write_global_wait:
	pause
	test dword [rdi], 1
	jnz ___rwlock_acquire_write_global_wait
	lock bts dword [rdi], 0
	jc ___rwlock_acquire_write_global_wait
	ret



__rwlock_release_write:
	lock btr dword [rdi], 0
	jmp scheduler_resume



__rwlock_acquire_read:
	push rdi
	call scheduler_pause
	pop rdi
	lock bts dword [rdi], 1
	jc ___rwlock_acquire_read_multiaccess_wait
	test dword [rdi], 4
	jnz ___rwlock_acquire_read_multiaccess_active
	jmp ___rwlock_acquire_read_global_test
___rwlock_acquire_read_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz ___rwlock_acquire_read_multiaccess_wait
	lock bts dword [rdi], 1
	jc ___rwlock_acquire_read_multiaccess_wait
	test dword [rdi], 4
	jnz ___rwlock_acquire_read_multiaccess_active
	jmp ___rwlock_acquire_read_global_test
___rwlock_acquire_read_global_wait:
	pause
	test dword [rdi], 1
	jnz ___rwlock_acquire_read_global_wait
___rwlock_acquire_read_global_test:
	lock bts dword [rdi], 0
	jc ___rwlock_acquire_read_global_wait
	bts dword [rdi], 2
___rwlock_acquire_read_multiaccess_active:
	add word [rdi], 8
	btr dword [rdi], 1
	ret



___rwlock_release_read_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz ___rwlock_release_read_multiaccess_wait
__rwlock_release_read:
	lock bts dword [rdi], 1
	jc ___rwlock_release_read_multiaccess_wait
	sub word [rdi], 8
	cmp word [rdi], 8
	jge ._still_used
	mov word [rdi], 0
	jmp scheduler_resume
._still_used:
	btr dword [rdi], 1
	jmp scheduler_resume



rwlock_is_held:
	test dword [rdi], 3
	setne al
	ret

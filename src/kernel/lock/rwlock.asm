;;; bit 0 - global lock
;;; bit 1 - multiaccess lock
;;; bit 2 - multiaccess active
;;; bits 3-31 - multiaccess counter



global rwlock_init:function default
global rwlock_acquire_write:function default
global rwlock_release_write:function default
global rwlock_acquire_read:function default
global rwlock_release_read:function default
global rwlock_is_held:function default
extern scheduler_disable_preemption
extern scheduler_enable_preemption
section .text exec nowrite



[bits 64]
rwlock_init:
	mov dword [rdi], 0
	ret


rwlock_acquire_write:
	call scheduler_disable_preemption
	lock bts dword [rdi], 0
	jc _rwlock_acquire_write_global_wait
	ret
_rwlock_acquire_write_global_wait:
	pause
	test dword [rdi], 1
	jnz _rwlock_acquire_write_global_wait
	lock bts dword [rdi], 0
	jc _rwlock_acquire_write_global_wait
	ret



rwlock_release_write:
	lock btr dword [rdi], 0
	jmp scheduler_enable_preemption



rwlock_acquire_read:
	call scheduler_disable_preemption
	jmp _rwlock_acquire_read_internal
_rwlock_acquire_read_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz _rwlock_acquire_read_multiaccess_wait
_rwlock_acquire_read_internal:
	lock bts dword [rdi], 1
	jc _rwlock_acquire_read_multiaccess_wait
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
	add word [rdi], 8
	btr dword [rdi], 1
	ret



_rwlock_release_read_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz _rwlock_release_read_multiaccess_wait
rwlock_release_read:
	lock bts dword [rdi], 1
	jc _rwlock_release_read_multiaccess_wait
	sub word [rdi], 8
	cmp word [rdi], 8
	jge ._still_used
	mov word [rdi], 0
	jmp scheduler_enable_preemption
._still_used:
	btr dword [rdi], 1
	jmp scheduler_enable_preemption



rwlock_is_held:
	test dword [rdi], 3
	setne al
	ret

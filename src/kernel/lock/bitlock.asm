global bitlock_init:function default
global bitlock_acquire:function default
global bitlock_try_acquire:function default
global bitlock_release:function default
global bitlock_is_held:function default
section .text exec nowrite



[bits 64]
bitlock_init:
	lock btr dword [rdi], esi
	ret



_bitlock_acquire_wait:
	pause
bitlock_acquire:
	lock bts dword [rdi], esi
	jc _bitlock_acquire_wait
	ret



bitlock_try_acquire:
	lock bts dword [rdi], esi
	setnc al
	ret



bitlock_release:
	lock btr dword [rdi], esi
	ret



bitlock_is_held:
	mov eax, dword [rdi]
	shrx eax, eax, esi
	and eax, 1
	ret

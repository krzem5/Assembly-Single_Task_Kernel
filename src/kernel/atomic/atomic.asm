global atomic_get
global atomic_set
global atomic_cas
section .text



[bits 64]
atomic_get:
	lock add dword [rdi], 0
	ret



atomic_set:
	lock xchg dword [rdi], esi
	ret



atomic_cas:
	mov rax, rsi
	lock cmpxchg dword [rdi], edx
	ret

%define MIN_ENTROPY_POOL_SIZE 128



global _random_entropy_pool
global _random_entropy_pool_length
global _random_init_entropy_pool
global _random_get_entropy
section .text



[bits 64]
_random_init_entropy_pool:
	xor ecx, ecx
._next_entry:
	rdtsc
	mov rdx, rax
	shl rdx, 32
	or rax, rdx
	xor rax, r8
	xor qword [_random_entropy_pool+rcx], rax
	add ecx, 1
	cmp ecx, 64
	jl ._next_entry
	;;; The loop above overruns into _random_entropy_pool_length, therefore it has to be initialized afterwards
	mov qword [_random_entropy_pool_length], MIN_ENTROPY_POOL_SIZE
	ret



_random_get_entropy:
	cmp qword [_random_entropy_pool_length], MIN_ENTROPY_POOL_SIZE
	jl ._not_enough_entropy
	mov qword [_random_entropy_pool_length], 0
	mov ecx, 8
	lea rsi, _random_entropy_pool
	rep movsq
._not_enough_entropy:
	ret



section .bss



align 8
_random_entropy_pool:
	resq 8
_random_entropy_pool_length:
	resq 1

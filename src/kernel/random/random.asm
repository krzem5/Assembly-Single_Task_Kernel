%define MIN_ENTROPY_POOL_SIZE 512



global _random_entropy_pool
global _random_entropy_pool_length
global _random_init_entropy_pool
global _random_has_entropy
global _random_get_entropy
section .text



[bits 64]
_random_init_entropy_pool:
	xor ecx, ecx
._next_entry:
	rdseed r8
	rdtsc
	mov rdx, rax
	shl rdx, 32
	or rax, rdx
	xor rax, r8
	xor qword [_random_entropy_pool+rcx], rax
	add ecx, 1
	cmp ecx, 128
	jl ._next_entry
	;;; The loop above overruns into _random_entropy_pool_length, therefore it has to be initialized afterwards
	mov qword [_random_entropy_pool_length], MIN_ENTROPY_POOL_SIZE
	ret



_random_has_entropy:
	xor eax, eax
	cmp qword [_random_entropy_pool_length], MIN_ENTROPY_POOL_SIZE
	jl ._no_entropy
	mov eax, 1
._no_entropy:
	ret



_random_get_entropy:
	mov qword [_random_entropy_pool_length], 0
	mov ecx, 16
	lea rsi, _random_entropy_pool
	rep movsq
	ret



section .data



align 8
_random_entropy_pool:
	times 16 dq 0
_random_entropy_pool_length:
	dq 0

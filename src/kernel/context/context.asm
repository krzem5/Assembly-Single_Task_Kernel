%macro WRITE_STATE_VALUE 2
	mov qword [rdi+(%1*8)], %2
%endmacro

%macro WRITE_STATE_VALUE_PTR 2
	mov rax, %2
	WRITE_STATE_VALUE %1,rax
%endmacro



global _context_get_cpu_state
section .text



[bits 64]
_context_get_cpu_state:
	mov rsi, qword [gs:8]
	sub rsi, 120
	WRITE_STATE_VALUE_PTR 0,qword [rsi+112] ; rip
	mov ecx, 0xc0000100
	rdmsr
	shl rdx, 32
	or rax, rdx
	WRITE_STATE_VALUE 1,rax ; fs
	mov ecx, 0xc0000102
	rdmsr
	shl rdx, 32
	or rax, rdx
	WRITE_STATE_VALUE 2,rax ; gs
	WRITE_STATE_VALUE_PTR 3,qword [rsi] ; rax
	WRITE_STATE_VALUE_PTR 4,qword [rsi+8] ; rbx
	WRITE_STATE_VALUE_PTR 5,qword [rsi+16] ; rdx
	WRITE_STATE_VALUE_PTR 6,qword [rsi+24] ; rsi
	WRITE_STATE_VALUE_PTR 7,qword [rsi+32] ; rdi
	WRITE_STATE_VALUE_PTR 8,qword [gs:16] ; rsp
	WRITE_STATE_VALUE_PTR 9,qword [rsi+40] ; rbp
	WRITE_STATE_VALUE_PTR 10,qword [rsi+48] ; r8
	WRITE_STATE_VALUE_PTR 11,qword [rsi+56] ; r9
	WRITE_STATE_VALUE_PTR 12,qword [rsi+64] ; r10
	WRITE_STATE_VALUE_PTR 13,qword [rsi+72] ; r12
	WRITE_STATE_VALUE_PTR 14,qword [rsi+80] ; r13
	WRITE_STATE_VALUE_PTR 15,qword [rsi+80] ; r14
	WRITE_STATE_VALUE_PTR 16,qword [rsi+80] ; r15
	add rdi, 136
	vmovdqu [rdi], ymm0
	vmovdqu [rdi+32], ymm1
	vmovdqu [rdi+64], ymm2
	vmovdqu [rdi+96], ymm3
	vmovdqu [rdi+128], ymm4
	vmovdqu [rdi+160], ymm5
	vmovdqu [rdi+192], ymm6
	vmovdqu [rdi+224], ymm7
	vmovdqu [rdi+256], ymm8
	vmovdqu [rdi+288], ymm9
	vmovdqu [rdi+320], ymm10
	vmovdqu [rdi+352], ymm11
	vmovdqu [rdi+384], ymm12
	vmovdqu [rdi+416], ymm13
	vmovdqu [rdi+448], ymm14
	vmovdqu [rdi+480], ymm15
	ret

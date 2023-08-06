extern _syscall_cpu_core_count
extern _syscall_cpu_core_start
extern _syscall_cpu_core_stop
extern main
global _start
section .text



[bits 64]
_start:
	call _syscall_cpu_core_count
	mov r12, rax
	mov r13, rax
	shr r13, 32
	xor r14, r14
._next_core:
	cmp r13, r14
	je ._skip_core
	mov rdi, r14
	lea rsi, _syscall_cpu_core_stop
	call _syscall_cpu_core_start
._skip_core:
	add r14, 1
	cmp r12, r14
	jne ._next_core
	call main
	jmp _syscall_cpu_core_stop

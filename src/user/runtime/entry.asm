%define CPU_DATA_SIZE_SHIFT 3
%define CPU_DATA_SIZE 8



%macro CALCULATE_CPU_DATA_POINTER_OFFSET 2
	mov %2, %1
	shl %2, CPU_DATA_SIZE_SHIFT
%endmacro

%macro CALCULATE_CPU_DATA_POINTER 2
	CALCULATE_CPU_DATA_POINTER_OFFSET %1, %2
	add %2, r15
%endmacro



extern _syscall_cpu_core_count
extern _syscall_cpu_core_start
extern _syscall_cpu_core_stop
extern _syscall_memory_map
extern main
global _start
section .text



[bits 64]
_start:
	;;; Acquire CPU count & BSP core id
	call _syscall_cpu_core_count
	mov r12, rax
	mov r13, rax
	mov r12d, r12d
	shr r13, 32
	xor r14, r14
	;;; Allocate per-core data
	CALCULATE_CPU_DATA_POINTER_OFFSET r12, rdi
	xor esi, esi
	call _syscall_memory_map
	mov r15, rax
	;;; Reset all cores & initialize gs bases
._next_core:
	CALCULATE_CPU_DATA_POINTER r14, rdx
	mov qword [rdx], r14
	cmp r13, r14
	je ._skip_core
	mov rdi, r14
	lea rsi, _start_core
	call _syscall_cpu_core_start
._skip_core:
	add r14, 1
	cmp r12, r14
	jne ._next_core
	CALCULATE_CPU_DATA_POINTER r13, rax
	wrgsbase rax
	;;; Start user code
	call main
	;;; Shutdown CPU
	jmp _syscall_cpu_core_stop



_start_core:
	wrgsbase rdi
	jmp _syscall_cpu_core_stop

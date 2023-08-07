%define CPU_DATA_SIZE_SHIFT 3



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
extern _clock_init
extern _cpu_init
extern _drive_init
extern _partition_init
extern _system_init
extern main
global _start
section .text



[bits 64]
_start:
	;;; Fix stack
	sub rsp, 8
	and rsp, 0xfffffffffffffff0
	mov rbp, rsp
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
	mov dword [_start_core_count], 1
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
	;;; Synchronize cores
._wait_for_cores:
	cmp dword [_start_core_count], r12d
	pause
	jne ._wait_for_cores
	;;; Call initializers
	call _clock_init
	call _cpu_init
	call _drive_init
	call _partition_init
	call _system_init
	;;; Start user code
	call main
	;;; Shutdown CPU
	jmp _syscall_cpu_core_stop



_start_core:
	wrgsbase rdi
	lock add dword [_start_core_count], 1
	jmp _syscall_cpu_core_stop



section .data



_start_core_count:
	dd 0

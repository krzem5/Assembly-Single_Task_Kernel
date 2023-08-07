extern _syscall_cpu_core_start
extern _syscall_cpu_core_stop
global cpu_core_start
section .text



[bits 64]
cpu_core_start:
	mov rcx, rdx
	mov rdx, rsi
	lea rsi, _cpu_core_bootstrap
	jmp _syscall_cpu_core_start



_cpu_core_bootstrap:
	;;; Fix stack
	sub rsp, 8
	and rsp, 0xfffffffffffffff0
	mov rbp, rsp
	;;; Call user function
	mov rax, rdi
	mov rdi, rsi
	call rax
	;;; Stop core
	jmp _syscall_cpu_core_stop

extern _syscall_cpu_core_start
extern _syscall_cpu_core_stop
global cpu_count
global cpu_bsp_id
global cpu_init
global cpu_core_start
global cpu_core_stop
section .text



[bits 64]
section .text.cpu_core_start
cpu_core_start:
	mov rcx, rdx
	mov rdx, rsi
	lea rsi, _cpu_core_bootstrap
	jmp _syscall_cpu_core_start
_cpu_core_bootstrap:
	sub rsp, 8
	and rsp, 0xfffffffffffffff0
	mov rbp, rsp
	mov rax, rdi
	mov rdi, rsi
	call rax
	jmp _syscall_cpu_core_stop



section .text.cpu_core_stop
cpu_core_stop:
	jmp _syscall_cpu_core_stop

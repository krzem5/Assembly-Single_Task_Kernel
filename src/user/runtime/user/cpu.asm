extern _syscall_cpu_core_count
extern _syscall_cpu_core_start
extern _syscall_cpu_core_stop
global cpu_count
global cpu_bsp_id
global cpu_init
global cpu_core_start
global cpu_core_stop
section .text



[bits 64]
section .text.cpu_init
cpu_init:
	call _syscall_cpu_core_count
	mov dword [cpu_count], eax
	shr rax, 32
	mov dword [cpu_bsp_id], eax
	ret



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



section .data.cpu_data



align 4
cpu_count:
	dd 0
cpu_bsp_id:
	dd 0

%define SYSCALL_COUNT 34



global syscall_enable
global syscall_jump_to_user_mode
extern vmm_common_kernel_pagemap
extern vmm_user_pagemap
extern _syscall_handlers
section .text



[bits 64]
syscall_enable:
	mov ecx, 0xc0000080
	rdmsr
	or eax, 1
	wrmsr
	xor eax, eax
	add ecx, 1
	mov edx, 0x00130008
	wrmsr
	lea rdx, syscall_handler
	mov rax, rdx
	add ecx, 1
	shr rdx, 32
	wrmsr
	mov eax, 0xfffffffd
	add ecx, 2
	xor edx, edx
	wrmsr
	ret



section .common



syscall_jump_to_user_mode:
	cmp qword [gs:32], 0
	pause
	jz syscall_jump_to_user_mode
	mov rcx, qword [gs:32]
	mov rdi, qword [gs:40]
	mov rsi, qword [gs:48]
	mov rsp, qword [gs:56]
	mov qword [gs:32], 0
	mov rax, qword [vmm_user_pagemap]
	mov cr3, rax
	mov ax, 0x18
	mov ds, ax
	mov es, ax
	xor eax, eax
	mov rbx, rax
	mov rdx, rax
	mov rbp, rsp
	mov r8, rax
	mov r9, rax
	mov r10, rax
	mov r11, rax
	mov r12, rax
	mov r13, rax
	mov r14, rax
	mov r15, rax
	vzeroall
	swapgs
	o64 sysret



syscall_handler:
	swapgs
	mov cr2, rsp
	mov sp, 0x10
	mov ds, sp
	mov es, sp
	mov rsp, qword [vmm_common_kernel_pagemap]
	mov cr3, rsp
	cmp qword [gs:32], 0
	jnz syscall_jump_to_user_mode
	mov rsp, cr2
	mov qword [gs:16], rsp
	mov rsp, qword [gs:8]
	push rcx
	push r11
	push r15
	push r14
	push r13
	push r12
	push r10
	push r9
	push r8
	push rbp
	push rdi
	push rsi
	push rdx
	push rbx
	push rax
	mov rdi, rsp
	cmp rax, SYSCALL_COUNT
	jl ._valid_syscall
	mov rsi, rax
	mov rax, SYSCALL_COUNT
._valid_syscall:
	mov rax, qword [_syscall_handlers+rax*8]
	cld
	call rax
	pop rax
	pop rbx
	pop rdx
	pop rsi
	pop rdi
	pop rbp
	pop r8
	pop r9
	pop r10
	pop r12
	pop r13
	pop r14
	pop r15
	pop r11
	pop rcx
	mov rsp, qword [gs:16]
	mov cr2, rsp
	mov rsp, qword [vmm_user_pagemap]
	mov cr3, rsp
	mov sp, 0x18
	mov ds, sp
	mov es, sp
	mov rsp, cr2
	swapgs
	o64 sysret

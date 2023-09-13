extern _random_entropy_pool
extern _random_entropy_pool_length
extern _syscall_handlers
extern syscall_invalid
extern user_data_cpu_table
extern vmm_common_kernel_pagemap
extern vmm_user_pagemap
global syscall_enable
global syscall_jump_to_user_mode
section .text



[bits 64]
syscall_enable:
	cmp qword [_syscall_count], 0
	jnz ._syscalls_counted
	mov rax, 0
._next_handler:
	add rax, 1
	cmp qword [_syscall_handlers+rax*8], 0
	jne ._next_handler
	mov qword [_syscall_count], rax
._syscalls_counted:
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
	jnz ._function_found
	sti
	hlt
	jmp syscall_jump_to_user_mode
._function_found:
	cli
	movzx eax, byte [gs:0]
	mov rdx, qword [user_data_cpu_table]
	mov rax, qword [rdx+rax*8]
	mov ecx, 0xc0000102
	mov rdx, rax
	shr rdx, 32
	wrmsr
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
	mov qword [gs:16], rsp
	mov rsp, qword [gs:8]
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rbp
	push rdi
	push rsi
	push rdx
	push rcx
	push rbx
	push rax
	mov rbx, qword [vmm_common_kernel_pagemap]
	mov cr3, rbx
	mov bx, 0x10
	mov ds, bx
	mov es, bx
	sti
	cmp qword [gs:32], 0
	jnz syscall_jump_to_user_mode._function_found
	mov rdi, rsp
	cmp rax, qword [_syscall_count]
	jge ._invalid_syscall
	mov rax, qword [_syscall_handlers+rax*8]
._call_syscall_handler:
	cld
	call rax
	cmp qword [gs:32], 0
	jnz syscall_jump_to_user_mode._function_found
	rdtsc
	xor rax, rdx
	mov edx, dword [_random_entropy_pool_length]
	and edx, 0x3c
	lock xor dword [_random_entropy_pool+rdx], eax
	cli
	mov rax, qword [vmm_user_pagemap]
	mov cr3, rax
	mov ax, 0x18
	mov ds, ax
	mov es, ax
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop rbp
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
	mov rsp, qword [gs:16]
	swapgs
	o64 sysret
._invalid_syscall:
	mov rax, syscall_invalid
	jmp ._call_syscall_handler



_syscall_count:
	dq 0

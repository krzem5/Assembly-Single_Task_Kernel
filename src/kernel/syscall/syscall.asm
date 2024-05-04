global syscall_enable:function
section .etext exec nowrite



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
	lea rdx, _syscall_handler
	mov rax, rdx
	add ecx, 1
	shr rdx, 32
	wrmsr
	mov eax, 0xfffffffd
	add ecx, 2
	xor edx, edx
	wrmsr
	ret



extern _random_entropy_pool
extern _random_entropy_pool_length
extern _syscall_invalid
extern _syscall_table_list
extern _syscall_table_list_length
extern scheduler_set_timer
section .text exec nowrite



[bits 64]
_syscall_handler:
	swapgs
	mov qword [gs:16], rsp
	mov rsp, qword [gs:8]
	push r15
	push r14
	push r13
	push r12
	push r11
	push rbp
	push rcx
	push rbx
	push r9
	push r8
	push r10
	push rdx
	push rsi
	push rdi
	mov bx, 0x10
	mov ds, bx
	mov es, bx
	mov r15, -0x0001 ; ERROR_INVALID_SYSCALL
	xor r14, r14
	xor r13, r13
	xor r12, r12
	xor r11, r11
	xor r10, r10
	xor rbp, rbp
	mov rbx, rax
	mov edi, 1 ; SCHEDULER_TIMER_KERNEL
	call scheduler_set_timer
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop r8
	pop r9
	mov eax, ebx
	shr rbx, 32
	cmp ebx, dword [_syscall_table_list_length]
	jge ._syscall_return
	mov r10, qword [_syscall_table_list]
	mov r10, qword [r10+rbx*8]
	test r10, r10
	jz ._syscall_return
	mov r11, qword [r10+8]
	cmp eax, dword [r10+16]
	jge ._syscall_return
	mov rax, qword [r11+rax*8]
	test rax, rax
	jz ._syscall_return
	sti
	call rax
	cli
	mov r15, rax
._syscall_return:
	xor edi, edi ; SCHEDULER_TIMER_USER
	call scheduler_set_timer
	rdtsc
	mov edx, dword [_random_entropy_pool_length]
	and edx, 0x3c
	lock xor dword [_random_entropy_pool+rdx], eax
	mov bx, 0x1b
	mov ds, bx
	mov es, bx
	mov rax, r15
	xor rdx, rdx
	xor rdi, rdi
	xor rsi, rsi
	xor r8, r8
	xor r9, r9
	xor r10, r10
	pop rbx
	pop rcx
	pop rbp
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
	mov rsp, qword [gs:16]
	swapgs
	o64 sysret

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
	xor ebx, ebx
	mov bx, ds
	push rbx
	mov bx, es
	push rbx
	mov bx, 0x10
	mov ds, bx
	mov es, bx
	xor rbp, rbp
	mov edi, 1 ; SCHEDULER_TIMER_KERNEL
	call scheduler_set_timer
	mov rax, qword [rsp+16]
	mov rdi, rsp
	test eax, eax
	jz _syscall_invalid
	mov rbx, rax
	shr rbx, 32
	mov eax, eax
	cmp ebx, dword [_syscall_table_list_length]
	jge _syscall_invalid
	mov rcx, qword [_syscall_table_list]
	mov rcx, qword [rcx+rbx*8]
	test rcx, rcx
	jz _syscall_invalid
	mov rdx, qword [rcx+8]
	cmp eax, dword [rcx+16]
	jz _syscall_invalid
	mov rax, qword [rdx+rax*8]
	test rax, rax
	jz _syscall_invalid
	call rax
._return_from_syscall:
	xor edi, edi ; SCHEDULER_TIMER_USER
	call scheduler_set_timer
	rdtsc
	mov edx, dword [_random_entropy_pool_length]
	and edx, 0x3c
	lock xor dword [_random_entropy_pool+rdx], eax
	pop rbx
	mov es, bx
	pop rbx
	mov ds, bx
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

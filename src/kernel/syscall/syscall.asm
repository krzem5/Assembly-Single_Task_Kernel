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
	;;;
	push r10
	push r9
	push r8
	push rdi
	push rsi
	push rdx
	;;;
	mov bx, 0x10
	mov ds, bx
	mov es, bx
	xor rbp, rbp
	xor r11, r11
	xor r12, r12
	xor r13, r13
	xor r14, r14
	mov r15, -0xaabb ; INVALID_SYSCALL_ERROR
	mov rbx, rax
	mov edi, 1 ; SCHEDULER_TIMER_KERNEL
	call scheduler_set_timer
	mov eax, ebx
	mov rdi, rsp
	shr rbx, 32
	cmp ebx, dword [_syscall_table_list_length]
	jge ._syscall_return
	mov rcx, qword [_syscall_table_list]
	mov rcx, qword [rcx+rbx*8]
	test rcx, rcx
	jz ._syscall_return
	mov rdx, qword [rcx+8]
	cmp eax, dword [rcx+16]
	jge ._syscall_return
	mov rax, qword [rdx+rax*8]
	test rax, rax
	jz ._syscall_return
	call rax
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
	;;;
	mov rax, r15
	pop rdx
	pop rsi
	pop rdi
	pop r8
	pop r9
	pop r10
	;;;
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

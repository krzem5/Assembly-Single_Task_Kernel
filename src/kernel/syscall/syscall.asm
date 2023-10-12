extern _random_entropy_pool
extern _random_entropy_pool_length
extern _syscall_count
extern _syscall_handlers
extern scheduler_set_timer
extern syscall_invalid
global syscall_enable
section .text exec nowrite



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
	mov bx, 0x10
	mov ds, bx
	mov es, bx
	xor rbp, rbp
	mov edi, 1
	call scheduler_set_timer
	mov rax, qword [rsp]
	mov rdi, rsp
	cmp rax, qword [_syscall_count]
	cmovge rax, rbp
	sti
	cld
	call qword [_syscall_handlers+rax*8]
	cli
	xor edi, edi
	call scheduler_set_timer
	rdtsc
	mov edx, dword [_random_entropy_pool_length]
	and edx, 0x3c
	lock xor dword [_random_entropy_pool+rdx], eax
	mov ax, 0x1b
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

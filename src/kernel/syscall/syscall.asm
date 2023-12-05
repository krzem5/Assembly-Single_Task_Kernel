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
extern _syscall_execute
section .text exec nowrite



[bits 64]
_syscall_handler:
	swapgs
	mov qword [gs:16], rsp
	mov rsp, qword [gs:8]
	push qword 0x1b
	push qword [gs:16]
	push r11
	push qword 0x23
	push rcx
	push qword 0
	push qword 0
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
	mov rdi, rsp
	call _syscall_execute
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
	add rsp, 16
	pop rcx
	add rsp, 8
	pop r11
	mov rsp, qword [rsp]
	swapgs
	o64 sysret

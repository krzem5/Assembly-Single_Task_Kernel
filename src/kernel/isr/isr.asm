extern idt_set_entry
extern _isr_handler
extern _isr_handler_inside_kernel
extern vmm_common_kernel_pagemap
extern vmm_user_pagemap
global isr_init
section .text



%macro REGISTER_ISR 1
	mov edi, %1
	lea rsi, isr%1
	xor edx, edx
	mov ecx, 0x8e
	call idt_set_entry
%endmacro



%macro ISR_HANDLER 1
isr%1:
	and rsp, 0xfffffffffffffff0
	push qword %1
	jmp _isr_common_handler
%endmacro



[bits 64]
isr_init:
	REGISTER_ISR 0
	REGISTER_ISR 1
	REGISTER_ISR 2
	REGISTER_ISR 3
	REGISTER_ISR 4
	REGISTER_ISR 5
	REGISTER_ISR 6
	REGISTER_ISR 7
	REGISTER_ISR 8
	REGISTER_ISR 9
	REGISTER_ISR 10
	REGISTER_ISR 11
	REGISTER_ISR 12
	REGISTER_ISR 13
	REGISTER_ISR 14
	REGISTER_ISR 15
	REGISTER_ISR 16
	REGISTER_ISR 17
	REGISTER_ISR 18
	REGISTER_ISR 19
	REGISTER_ISR 20
	REGISTER_ISR 21
	REGISTER_ISR 22
	REGISTER_ISR 23
	REGISTER_ISR 24
	REGISTER_ISR 25
	REGISTER_ISR 26
	REGISTER_ISR 27
	REGISTER_ISR 28
	REGISTER_ISR 29
	REGISTER_ISR 30
	REGISTER_ISR 31
	ret



section .common



_isr_common_handler:
	mov rsp, cr3
	cmp rsp, qword [vmm_common_kernel_pagemap]
	je ._inside_kernel
	swapgs
	mov sp, 0x10
	mov ds, sp
	mov es, sp
	mov ss, sp
	mov rsp, qword [vmm_common_kernel_pagemap]
	mov cr3, rsp
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
	mov rax, cr4
	push rax
	mov rax, cr3
	push rax
	mov rax, cr2
	push rax
	mov rax, cr0
	push rax
	mov rax, qword [gs:32]
	push qword [rax-48]
	mov rdi, rsp
	mov rsi, qword [rax-56]
	lea rdx, [rax-40]
	cld
	jmp _isr_handler
._inside_kernel:
	mov rsp, qword [gs:8]
	cld
	jmp _isr_handler_inside_kernel



ISR_HANDLER 0
ISR_HANDLER 1
ISR_HANDLER 2
ISR_HANDLER 3
ISR_HANDLER 4
ISR_HANDLER 5
ISR_HANDLER 6
ISR_HANDLER 7
ISR_HANDLER 8
ISR_HANDLER 9
ISR_HANDLER 10
ISR_HANDLER 11
ISR_HANDLER 12
ISR_HANDLER 13
ISR_HANDLER 14
ISR_HANDLER 15
ISR_HANDLER 16
ISR_HANDLER 17
ISR_HANDLER 18
ISR_HANDLER 19
ISR_HANDLER 20
ISR_HANDLER 21
ISR_HANDLER 22
ISR_HANDLER 23
ISR_HANDLER 24
ISR_HANDLER 25
ISR_HANDLER 26
ISR_HANDLER 27
ISR_HANDLER 28
ISR_HANDLER 29
ISR_HANDLER 30
ISR_HANDLER 31

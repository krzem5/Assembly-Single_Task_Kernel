extern idt_set_entry
extern _isr_handler
extern _isr_kernel
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



_isr_handler_internal:
	cmp qword [rsp+24], 0x23
	jne ._kernel_isr
	swapgs
	mov sp, 0x10
	mov ds, sp
	mov es, sp
	mov ss, sp
	mov rsp, qword [vmm_common_kernel_pagemap]
	mov cr3, rsp
	mov rsp, qword [gs:8]
	sub rsp, 0x38
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
	mov rdi, rsp
	cld
	call _isr_handler
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
	mov rsp, qword [vmm_user_pagemap]
	mov cr3, rsp
	mov sp, 0x18
	mov ds, sp
	mov es, sp
	mov rsp, qword [gs:8]
	sub rsp, 0x28
	swapgs
	iretq
._kernel_isr:
	mov rdi, qword [rsp]
	jmp _isr_kernel



isr0:
	push qword 0x00
	push qword 0x00
	jmp _isr_handler_internal



isr1:
	push qword 0x00
	push qword 0x01
	jmp _isr_handler_internal



isr2:
	push qword 0x00
	push qword 0x02
	jmp _isr_handler_internal



isr3:
	push qword 0x00
	push qword 0x03
	jmp _isr_handler_internal



isr4:
	push qword 0x00
	push qword 0x04
	jmp _isr_handler_internal



isr5:
	push qword 0x00
	push qword 0x05
	jmp _isr_handler_internal



isr6:
	push qword 0x00
	push qword 0x06
	jmp _isr_handler_internal



isr7:
	push qword 0x00
	push qword 0x07
	jmp _isr_handler_internal



isr8:
	push qword 0x08
	jmp _isr_handler_internal



isr9:
	push qword 0x00
	push qword 0x09
	jmp _isr_handler_internal



isr10:
	push qword 0x0a
	jmp _isr_handler_internal



isr11:
	push qword 0x0b
	jmp _isr_handler_internal



isr12:
	push qword 0x0c
	jmp _isr_handler_internal



isr13:
	push qword 0x0d
	jmp _isr_handler_internal



isr14:
	push qword 0x0e
	jmp _isr_handler_internal



isr15:
	push qword 0x00
	push qword 0x0f
	jmp _isr_handler_internal



isr16:
	push qword 0x00
	push qword 0x10
	jmp _isr_handler_internal



isr17:
	push qword 0x00
	push qword 0x11
	jmp _isr_handler_internal



isr18:
	push qword 0x00
	push qword 0x12
	jmp _isr_handler_internal



isr19:
	push qword 0x00
	push qword 0x13
	jmp _isr_handler_internal



isr20:
	push qword 0x00
	push qword 0x14
	jmp _isr_handler_internal



isr21:
	push qword 0x00
	push qword 0x15
	jmp _isr_handler_internal



isr22:
	push qword 0x00
	push qword 0x16
	jmp _isr_handler_internal



isr23:
	push qword 0x00
	push qword 0x17
	jmp _isr_handler_internal



isr24:
	push qword 0x00
	push qword 0x18
	jmp _isr_handler_internal



isr25:
	push qword 0x00
	push qword 0x19
	jmp _isr_handler_internal



isr26:
	push qword 0x00
	push qword 0x1a
	jmp _isr_handler_internal



isr27:
	push qword 0x00
	push qword 0x1b
	jmp _isr_handler_internal



isr28:
	push qword 0x00
	push qword 0x1c
	jmp _isr_handler_internal



isr29:
	push qword 0x00
	push qword 0x1d
	jmp _isr_handler_internal



isr30:
	push qword 0x00
	push qword 0x1e
	jmp _isr_handler_internal



isr31:
	push qword 0x00
	push qword 0x1f
	jmp _isr_handler_internal

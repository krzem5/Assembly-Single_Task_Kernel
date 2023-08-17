extern _isr_handler
extern _isr_handler_inside_kernel
extern _lapic_registers
extern _random_entropy_pool
extern _random_entropy_pool_length
extern idt_set_entry
extern vmm_common_kernel_pagemap
extern vmm_user_pagemap
global isr_allocate
global isr_was_triggered
global isr_wait
section .text



[bits 64]
isr_allocate:
	movzx ecx, byte [_next_irq_index]
	cmp ecx, 0xfe
	jge $
	mov eax, ecx
	add ecx, 1
	mov byte [_next_irq_index], cl
	ret



isr_was_triggered:
	xor eax, eax
	mov edx, 1
	mov rsi, rdi
	and edi, 31
	shr rsi, 5
	lea rsi, [_isr_mask+rsi*4]
	lock btr dword [rsi], edi
	cmovc eax, edx
	ret



isr_wait:
	mov rsi, rdi
	and edi, 31
	shr rsi, 5
	lea rsi, [_isr_mask+rsi*4]
._retry:
	lock btr dword [rsi], edi
	jc ._end
	cmp qword [gs:32], 0
	jne ._fail
	hlt
	jmp ._retry
._end:
	mov eax, 1
	ret
._fail:
	xor eax, eax
	ret



_irq_common_handler:
	cli
	push rdx
	push rax
	mov rax, qword [_lapic_registers]
	mov rdx, qword [rsp+16]
	mov dword [rax+0x0b0], 0
	cmp edx, 0xfe
	je ._skip_ipi_wakeup
	mov dword [rax+0x300], 0x000c00fe
	mov rax, rdx
	shr rdx, 5
	and rax, 31
	lock bts qword [_isr_mask+rdx*4], rax
	rdtsc
	xor eax, dword [rsp+24] ; rip
	xor eax, dword [rsp+48] ; rsp
	mov rdx, 4
	lock xadd qword [_random_entropy_pool_length], rdx
	and rdx, 0x3c
	lock xor dword [_random_entropy_pool+rdx], eax
._skip_ipi_wakeup:
	pop rax
	pop rdx
	add rsp, 8
	sti
_isr_entry_255:
	iretq



%assign idx 32
%rep 223
_isr_entry_%+idx:
	push qword idx
	jmp _irq_common_handler
%assign idx idx+1
%endrep



section .data


align 4
_next_irq_index:
	dd 32



section .bss



align 4
_isr_mask:
	resd 8



section .common



_isr_common_handler:
	pop rax
	jmp $
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
	mov rax, qword [gs:24]
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



%assign idx 0
%rep 32
_isr_entry_%+idx:
	and rsp, 0xfffffffffffffff0
	push qword idx
	jmp _isr_common_handler
%assign idx idx+1
%endrep

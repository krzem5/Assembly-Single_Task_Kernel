extern _isr_handler
extern _isr_handler_inside_kernel
extern _lapic_registers
extern idt_set_entry
extern vmm_common_kernel_pagemap
extern vmm_user_pagemap
global isr_init
global isr_allocate
global isr_wait
section .text



%macro ISR_HANDLER 1
isr%1:
	and rsp, 0xfffffffffffffff0
	push qword %1
	jmp _isr_common_handler
%endmacro



[bits 64]
isr_init:
%assign idx 0
%rep 256
	mov edi, idx
	lea rsi, isr%+idx
	mov ecx, 0x8e
	xor edx, edx
	call idt_set_entry
%assign idx idx+1
%endrep
	mov byte [_next_isr_index],32
	ret



isr_allocate:
	movzx ecx, byte [_next_isr_index]
	cmp ecx, 0xfe
	jge $
	mov eax, ecx
	add ecx, 1
	mov byte [_next_isr_index], cl
	ret



isr_wait:
	mov dl, dl
	mov rsi, rdi
	and edi, 31
	shr rsi, 5
	lea rsi, [_isr_mask+rsi*4]
._retry:
	lock btr dword [rsi], edi
	jc ._end
	hlt
	jmp ._retry
._end:
	ret



%assign idx 32
%rep 222
isr%+idx:
	mov dword [_lapic_registers+0x0b0], 0
	mov dword [_lapic_registers+0x300], 0x000c00fe
	lock bts qword [_isr_mask+(idx>>5)*4], (idx&31)
	iretq
%assign idx idx+1
%endrep



isr254:
	mov dword [_lapic_registers+0x0b0], 0
	iretq



isr255:
	iretq



section .data



_next_isr_index:
	db 0
align 4
_isr_mask:
	times 8 dd 0



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
	jmp $
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

extern _isr_handler
extern _random_entropy_pool
extern _random_entropy_pool_length
global isr_allocate
section .text exec nowrite



[bits 64]
isr_allocate:
	movzx ecx, byte [_next_irq_index]
	cmp ecx, 0xfe
	jge $
	mov eax, ecx
	add ecx, 1
	mov byte [_next_irq_index], cl
	ret



_isr_common_handler:
	cmp qword [rsp+24], 0x08
	je ._kernel_entry
	swapgs
._kernel_entry:
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
	xor rax, rax
	mov ax, ds
	push rax
	mov ax, es
	push rax
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	xor rbp, rbp
	mov rdi, rsp
	cld
	call _isr_handler
	rdtsc
	xor eax, dword [rsp+160] ; rip
	xor eax, dword [rsp+184] ; rsp
	mov rdx, 4
	lock xadd qword [_random_entropy_pool_length], rdx
	and rdx, 0x3c
	lock xor dword [_random_entropy_pool+rdx], eax
	pop rax
	mov es, ax
	pop rax
	mov ds, ax
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
	cmp qword [rsp+8], 0x08
	je ._kernel_exit
	swapgs
._kernel_exit:
_isr_entry_255:
	iretq



%assign idx 0
%rep 255
_isr_entry_%+idx:
	cli
%if idx!=8&&idx!=10&&idx!=11&&idx!=12&&idx!=13&&idx!=14&&idx!=17&&idx!=30
	push qword 0
%endif
	push qword idx
	jmp _isr_common_handler
%assign idx idx+1
%endrep



section .data noexec write



align 4
_next_irq_index:
	dd 33

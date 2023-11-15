global gdt_enable:function
section .text exec nowrite



[bits 64]
gdt_enable:
	mov rax, 0x0000890000000067
	mov rcx, rdi
	and rcx, 0xffffff
	shl rcx, 16
	or rax, rcx
	mov rcx, rdi
	and ecx, 0xff000000
	shl rcx, 32
	or rax, rcx
	mov qword [gdt_tss], rax
	shr rdi, 32
	mov qword [gdt_tss+8], rdi
	lgdt [gdt_pointer]
	push (gdt_kernel_code-gdt_start)
	push ._update_cs
	retfq
._update_cs:
	xor eax, eax
	mov ax, (gdt_kernel_data-gdt_start)
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov ax, (gdt_user_data-gdt_start)
	mov fs, ax
	mov gs, ax
	mov ax, (gdt_tss-gdt_start)
	ltr ax
	ret



section .data noexec write



align 16
gdt_start:
	dq 0x0000000000000000
gdt_kernel_code:
	dq 0x00209a0000000000
gdt_kernel_data:
	dq 0x0000920000000000
gdt_user_data:
	dq 0x0000f20000000000
gdt_user_code:
	dq 0x0020fa0000000000
gdt_tss:
	dq 0x0000000000000000
	dq 0x0000000000000000
gdt_end:



section .rdata noexec nowrite



gdt_pointer:
	dw gdt_end-gdt_start-1
	dq gdt_start

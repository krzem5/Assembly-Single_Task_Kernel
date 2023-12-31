global gdt_enable:function hidden
section .etext exec nowrite



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
	mov qword [gdt.tss], rax
	shr rdi, 32
	mov qword [gdt.tss+8], rdi
	lgdt [gdt_pointer]
	push (gdt.kernel_code-gdt)
	push ._update_cs
	retfq
._update_cs:
	xor eax, eax
	mov ax, (gdt.kernel_data-gdt)
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov ax, (gdt.user_data-gdt)
	mov fs, ax
	mov gs, ax
	mov ax, (gdt.tss-gdt)
	ltr ax
	ret



section .idata noexec write



align 16
gdt:
	dq 0x0000000000000000
.kernel_code:
	dq 0x00209b0000000000
.kernel_data:
	dq 0x0000930000000000
.user_data:
	dq 0x0000f30000000000
.user_code:
	dq 0x0020fb0000000000
.tss:
	dq 0x0000000000000000
	dq 0x0000000000000000
.end equ $



section .erdata noexec nowrite



gdt_pointer:
	dw gdt.end-gdt-1
	dq gdt

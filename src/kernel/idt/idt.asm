extern _idt_data
global idt_enable
global idt_set_entry
section .text



[bits 64]
idt_enable:
	mov rax, qword [_idt_data]
	mov qword [idt_pointer._data], rax
	lidt [idt_pointer]
	ret



idt_set_entry:
	mov r8, qword [_idt_data]
	shl rdi, 4
	mov rax, rsi
	shr rax, 32
	mov qword [r8+rdi+8], rax
	mov rax, rsi
	and eax, 0x0000ffff
	and esi, 0xffff0000
	shl rsi, 32
	or rax, rsi
	shl rdx, 32
	or rax, rdx
	shl rcx, 40
	or rax, rcx
	or rax, 0x00080000
	mov qword [r8+rdi], rax
	ret



section .data



align 16
idt_pointer:
	dw 0x0fff
._data:
	dq 0

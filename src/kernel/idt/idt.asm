extern _idt_data
global idt_enable
section .text



[bits 64]
idt_enable:
	mov rax, qword [_idt_data]
	mov qword [idt_pointer._data], rax
	lidt [idt_pointer]
	ret



section .data



align 16
idt_pointer:
	dw 0x0fff
._data:
	dq 0

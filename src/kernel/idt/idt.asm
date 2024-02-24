global _idt_set_data_pointer:function hidden
global idt_enable:function hidden
section .etext exec nowrite



[bits 64]
_idt_set_data_pointer:
	mov qword [idt_pointer.data], rdi
	ret



idt_enable:
	lidt [idt_pointer]
	ret



section .edata noexec write



align 16
idt_pointer:
	dw 0x0fff
.data:
	dq 0

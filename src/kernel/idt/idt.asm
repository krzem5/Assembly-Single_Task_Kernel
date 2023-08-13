global _idt_data
global idt_enable
section .text



[bits 64]
idt_enable:
	lidt [idt_pointer]
	ret



section .data



align 16
idt_pointer:
	dw 0x0fff
._data:
	dq _idt_data



section .common



align 8
_idt_data:
	times 512 dq 0

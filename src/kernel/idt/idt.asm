global idt_enable:function
section .text exec nowrite



[bits 64]
idt_enable:
	lidt [idt_pointer]
	ret



section .rdata noexec nowrite



align 16
idt_pointer:
	dw 0x0fff
	dq _idt_data
align 8
_idt_data:
	times 512 dq 0

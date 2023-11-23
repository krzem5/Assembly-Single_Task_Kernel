global idt_enable:function hidden
section .etext exec nowrite



[bits 64]
idt_enable:
	lidt [idt_pointer]
	ret



section .erdata noexec nowrite



align 16
idt_pointer:
	dw 0x0fff
	dq _idt_data



section .rdata noexec nowrite



align 8
_idt_data:
	times 512 dq 0

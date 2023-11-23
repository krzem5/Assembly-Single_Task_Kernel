global _shutdown_restart:function hidden
section .text exec nowrite



[bits 64]
_shutdown_restart:
	lidt [_empty_idt_pointer]
	sti
	int 1
	ret



section .rdata noexec nowrite



_empty_idt_pointer:
	dw 0x0000
	dq 0x0000000000000000

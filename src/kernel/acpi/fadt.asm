global _acpi_fadt_reboot
section .text exec nowrite



[bits 64]
_acpi_fadt_reboot:
	lidt [empty_idt_pointer]
	sti
	int 1
	ret



section .rdata noexec nowrite



empty_idt_pointer:
	dw 0x0000
	dq 0x0000000000000000

global enable_paging
section .text exec nowrite



[bits 64]
enable_paging:
	mov rax, cr4
	or rax, 0x20
	mov cr4, rax
	ret

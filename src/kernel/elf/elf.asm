global elf_get_hwcap:function hidden
section .etext exec nowrite



[bits 64]
elf_get_hwcap:
	push rbx
	mov eax, 1
	cpuid
	mov eax, edx
	pop rbx
	ret

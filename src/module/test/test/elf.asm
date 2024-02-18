global test_elf_get_correct_hwcap:function hidden
section .text exec nowrite



[bits 64]
test_elf_get_correct_hwcap:
	push rbx
	mov eax, 1
	cpuid
	mov eax, edx
	pop rbx
	ret

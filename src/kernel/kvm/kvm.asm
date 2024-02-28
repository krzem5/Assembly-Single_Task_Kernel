global _kvm_get_flags:function hidden
section .etext exec nowrite



[bits 64]
_kvm_get_flags:
	push rbx
	mov eax, 0x00000001
	cpuid
	test ecx, 0x80000000
	jz ._not_kvm
	mov eax, 0x40000000
	cpuid
	cmp ebx, 0x4b4d564b
	jne ._not_kvm
	cmp ecx, 0x564b4d56
	jne ._not_kvm
	cmp edx, 0x0000004d
	jne ._not_kvm
	mov eax, 0x40000001
	cpuid
	mov dword [rdi], eax
	mov dword [rsi], edx
	pop rbx
	mov eax, 1
	ret
._not_kvm:
	pop rbx
	xor eax, eax
	ret

global topology_get_cpu_bits
section .text



[bits 64]
topology_get_cpu_bits:
	push rbx
	xor eax, eax
	cpuid
	cmp ebx, 0x756e6547
	jne ._error
	cmp edx, 0x49656e69
	jne ._error
	cmp ecx, 0x6c65746e
	jne ._error
	cmp eax, 11
	jge ._intel_11
	cmp eax, 4
	jge ._intel_04
._error:
	xor rax, rax
	pop rbx
	ret
._intel_11:
	mov eax, 11
	xor ecx, ecx
	cpuid
	mov r8, rax
	mov eax, 11
	mov ecx, 1
	cpuid
	sub rax, r8
	shl rax, 32
	or rax, r8
	pop rbx
	ret
._intel_04:
	pop rbx
	ret

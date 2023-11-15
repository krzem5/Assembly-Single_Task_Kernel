global topology_get_cpu_bits:function
section .text exec nowrite



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
	cmp eax, 1
	jge ._intel_01
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
	mov eax, 4
	cpuid
	shr eax, 26
	bsr r8d, eax
	add r8, 1
	mov eax, 1
	cpuid
	shr eax, 16
	sub eax, 1
	and eax, 0xff
	bsr eax, eax
	add eax, 1
	sub rax, r8
	shl r8, 32
	or rax, r8
	pop rbx
	ret
._intel_01:
	mov eax, 1
	cpuid
	mov eax, ebx
	shr eax, 16
	sub eax, 1
	and eax, 0xff
	bsr eax, eax
	add eax, 1
	pop rbx
	ret

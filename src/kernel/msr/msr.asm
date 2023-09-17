global msr_get_apic_id
global msr_enable_apic
global msr_get_fs_base
global msr_get_gs_base
global msr_set_fs_base
global msr_set_gs_base
global msr_enable_simd
global msr_enable_rdtsc
global msr_enable_fsgsbase
section .text



[bits 64]
msr_get_apic_id:
	push rbx
	mov eax, 1
	cpuid
	mov eax, ebx
	shr eax, 24
	pop rbx
	ret



msr_enable_apic:
	mov ecx, 0x0000001b
	rdmsr
	bts eax, 11
	wrmsr
	ret



msr_get_fs_base:
	mov ecx, 0xc0000100
	rdmsr
	shr rdx, 32
	or rax, rdx
	ret



msr_get_gs_base:
	mov ecx, 0xc0000101
	and edi, 1
	add ecx, edi
	rdmsr
	shr rdx, 32
	or rax, rdx
	ret



msr_set_fs_base:
	mov rdx, rdi
	mov eax, edi
	mov ecx, 0xc0000100
	shr rdx, 32
	wrmsr
	ret



msr_set_gs_base:
	and esi, 1
	mov rdx, rdi
	mov eax, edi
	mov ecx, 0xc0000101
	add ecx, esi
	shr rdx, 32
	wrmsr
	ret



msr_enable_simd:
	push rbx
	mov rax, cr0
	and rax, 0xfffffffffffffffb
	or rax, 0x00000002
	mov cr0, rax
	mov rax, cr4
	or rax, 0x00040600
	mov cr4, rax
	mov eax, 1
	cpuid
	shr ecx, 26
	and ecx, 4
	mov ebx, ecx
	xor ecx, ecx
	xgetbv
	or eax, 0x00000003
	or eax, ebx
	xsetbv
	mov eax, 13
	cpuid
	mov eax, ecx
	pop rbx
	ret



msr_enable_rdtsc:
	mov rax, cr4
	and rax, 0xfffffffffffffffb
	mov cr4, rax
	ret



msr_enable_fsgsbase:
	mov rax, cr4
	or rax, 0x00010000
	mov cr4, rax
	ret

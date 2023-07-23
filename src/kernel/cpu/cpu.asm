global cpu_get_apic_id
global cpu_set_fs_base
global cpu_set_gs_base
global cpu_enable_simd
section .text



[bits 64]
cpu_get_apic_id:
	push rbx
	mov eax, 1
	cpuid
	mov eax, ebx
	shr eax, 24
	pop rbx
	ret



cpu_set_fs_base:
	mov rdx, rdi
	mov eax, edi
	mov ecx, 0xc0000100
	shr rdx, 32
	wrmsr
	ret



cpu_set_gs_base:
	and esi, 1
	mov rdx, rdi
	mov eax, edi
	mov ecx, 0xc0000101
	add ecx, esi
	shr rdx, 32
	wrmsr
	ret



cpu_enable_simd:
	mov rax, cr0
	and rax, 0xfffffffffffffffb
	or rax, 0x00000002
	mov cr0, rax
	mov rax, cr4
	or rax, 0x00040600
	mov cr4, rax
	xor ecx, ecx
	xgetbv
	or eax, 0x00000007
	xsetbv
	vzeroall
	ret

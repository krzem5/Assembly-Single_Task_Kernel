global msr_get_apic_id:function hidden
global msr_enable_apic:function hidden
global msr_set_fs_base:function hidden
global msr_set_gs_base:function hidden
global msr_enable_rdtsc:function hidden
section .text exec nowrite



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



msr_enable_rdtsc:
	mov rax, cr4
	and rax, 0xfffffffffffffffb
	mov cr4, rax
	ret

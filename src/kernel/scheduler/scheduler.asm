global scheduler_task_wait_loop
global scheduler_init_fpu
global scheduler_save_fpu
global scheduler_restore_fpu
extern cpu_fpu_state_size
extern memset
section .text



[bits 64]
scheduler_task_wait_loop:
	sti
	hlt
	jmp scheduler_task_wait_loop



scheduler_init_fpu:
	push rdi
	xor esi, esi
	mov edx, dword [cpu_fpu_state_size]
	call memset
	mov rdi, qword [rsp]
	vzeroall
	mov dword [rsp], 0x37f
	fldcw [rsp]
	mov dword [rsp], 0x1f80
	ldmxcsr [rsp]
	xor eax, eax
	sub eax, 1
	mov edx, eax
	xsave [rdi]
	add rsp, 8
	ret



scheduler_save_fpu:
	xor eax, eax
	sub eax, 1
	mov edx, eax
	xsave [rdi]
	ret



scheduler_restore_fpu:
	xor eax, eax
	sub eax, 1
	mov edx, eax
	xrstor [rdi]
	ret

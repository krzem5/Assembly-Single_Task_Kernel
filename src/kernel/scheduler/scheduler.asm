global scheduler_task_wait_loop
global scheduler_init_fpu
global scheduler_save_fpu
global scheduler_restore_fpu
section .text



[bits 64]
scheduler_task_wait_loop:
	sti
	hlt
	jmp scheduler_task_wait_loop



scheduler_init_fpu:
	sub rsp, 8
	vzeroall
	mov dword [rsp], 0b1100111111
	fldcw [rsp]
	mov dword [rsp], 0b1111110000000
	ldmxcsr [rsp]
	xor eax, eax
	sub eax, 1
	mov edx, eax
	xsave [rdi]
	add rsp, 8
	ret



scheduler_save_fpu:
	jmp $
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

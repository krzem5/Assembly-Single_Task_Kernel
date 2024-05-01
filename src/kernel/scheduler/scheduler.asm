global scheduler_yield:function default
global scheduler_task_wait_loop:function default
global scheduler_disable_preemption:function default
global scheduler_enable_preemption:function default
extern _scheduler_preemption_disabled
section .text exec nowrite



[bits 64]
scheduler_yield:
	pushfq
	sti
	int 32
	pop rax
	test rax, 512
	jnz ._interrupts_enabled
	cli
._interrupts_enabled:
	ret



scheduler_task_wait_loop:
	sti
	hlt
	int 32
	jmp scheduler_task_wait_loop



scheduler_disable_preemption:
	mov rax, qword [_scheduler_preemption_disabled]
	add rax, qword [gs:0]
	add dword [rax], 1
	ret



scheduler_enable_preemption:
	mov rax, qword [_scheduler_preemption_disabled]
	add rax, qword [gs:0]
	sub dword [rax], 1
	ret

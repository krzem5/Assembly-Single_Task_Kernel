global scheduler_yield:function default
global scheduler_task_wait_loop:function default
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
	jmp scheduler_task_wait_loop

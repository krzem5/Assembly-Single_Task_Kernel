global scheduler_yield
global scheduler_task_wait_loop
section .text exec nowrite



[bits 64]
scheduler_yield:
	pushfq
	pop rax
	sti
	int 32
	test rax, 512
	jnz ._interrupts_enabled
	cli
._interrupts_enabled:
	ret



scheduler_task_wait_loop:
	sti
	hlt
	jmp scheduler_task_wait_loop

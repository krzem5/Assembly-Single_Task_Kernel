global scheduler_start
global scheduler_task_wait_loop
section .text



[bits 64]
scheduler_start:
	sti
	int 32
	ret



scheduler_task_wait_loop:
	sti
	hlt
	jmp scheduler_task_wait_loop

global scheduler_yield:function default
global scheduler_task_wait_loop:function default
global scheduler_disable_preemption:function default
global scheduler_enable_preemption:function default
extern _scheduler_ensure_no_locks
extern _scheduler_preemption_disabled
section .text exec nowrite



[bits 64]
scheduler_yield:
	call _scheduler_ensure_no_locks
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



extern scheduler_pause
scheduler_disable_preemption:
	push rdi
	call scheduler_pause
	pop rdi
	ret
	; cli
	; mov rax, qword [_scheduler_preemption_disabled]
	; mov rcx, qword [gs:0]
	; add dword [rax+rcx*4], 1
	; ret



extern scheduler_resume
scheduler_enable_preemption:
	push rdi
	call scheduler_resume
	pop rdi
	ret
; 	mov rax, qword [_scheduler_preemption_disabled]
; 	mov rcx, qword [gs:0]
; 	sub dword [rax+rcx*4], 1
; 	cmp dword [rax+rcx*4], 0
; 	jnz ._skip_enable_interrupts
; 	sti
; ._skip_enable_interrupts:
; 	ret

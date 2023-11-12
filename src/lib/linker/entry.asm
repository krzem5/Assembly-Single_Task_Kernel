extern _syscall_thread_stop
extern main
global _start
section .text exec nowrite



[bits 64]
[default rel]
_start:
	sub rsp, 8
	and rsp, 0xfffffffffffffff0
	mov rbp, rsp
	call [rel main wrt ..got]
	jmp [rel _syscall_thread_stop wrt ..got]

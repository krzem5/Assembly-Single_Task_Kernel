extern _syscall_thread_stop
extern _user_data_init
extern main
global _start
section .text exec nowrite



[bits 64]
_start:
	;;; Fix stack
	sub rsp, 8
	and rsp, 0xfffffffffffffff0
	mov rbp, rsp
	;;; Initialize system data
	call _user_data_init
	;;; Start user code
	call main
	;;; Shutdown CPU
	jmp _syscall_thread_stop

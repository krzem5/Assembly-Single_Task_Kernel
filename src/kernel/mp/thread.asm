extern thread_terminate
global _thread_bootstrap_kernel_thread:function hidden
section .text exec nowrite



[bits 64]
_thread_bootstrap_kernel_thread:
	call rax
	jmp thread_terminate

%include "sys/types.inc"
extern _syscall_thread_stop
extern main
global _start:function hidden
section .text exec nowrite



[bits 64]
_start:
	; argc    = r15[0]
	; argv    = r15+1
	; environ = r15+1+argc
	; auxv    = r15+1+argc+#environ+1
	mov edi, dword [r15]
	lea rsi, qword [r15+8]
	lea rdx, qword [rsi+rdi*8]
	mov rcx, rdx
._skip_entry:
	cmp qword [rcx], 0
	je ._found_environ_end
	add rcx, 8
	jmp ._skip_entry
._found_environ_end:
	add rcx, 8
	call [REF(main)]
	jmp [REF(_syscall_thread_stop)]

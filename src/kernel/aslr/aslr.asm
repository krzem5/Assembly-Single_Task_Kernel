global _aslr_adjust_rip:function hidden
section .etext exec nowrite



[bits 64]
_aslr_adjust_rip:
	xor rbp, rbp
	jmp rsi

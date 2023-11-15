global pf_get_fault_address:function
global pf_invalidate_tlb_entry:function
section .text exec nowrite



[bits 64]
pf_get_fault_address:
	mov rax, cr2
	ret



pf_invalidate_tlb_entry:
	invlpg [rdi]
	ret

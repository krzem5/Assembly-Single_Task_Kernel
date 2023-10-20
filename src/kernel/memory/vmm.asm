global vmm_get_fault_address
global vmm_get_pagemap
global vmm_invalidate_tlb_entry
global vmm_switch_to_pagemap
section .text exec nowrite



[bits 64]
vmm_get_fault_address:
	mov rax, cr2
	ret



vmm_get_pagemap:
	mov rax, cr3
	mov qword [rdi], rax
	ret




vmm_invalidate_tlb_entry:
	invlpg [rdi]
	ret



vmm_switch_to_pagemap:
	mov rax, qword [rdi]
	mov cr3, rax
	ret

global vmm_get_fault_address
global vmm_invalidate_tlb_entry
global vmm_switch_to_pagemap
global vmm_set_common_kernel_pagemap
global vmm_common_kernel_pagemap
extern vmm_kernel_pagemap
section .ctext



[bits 64]
vmm_get_fault_address:
	mov rax, cr2
	ret



vmm_invalidate_tlb_entry:
	invlpg [rdi]
	ret



vmm_switch_to_pagemap:
	mov rax, qword [rdi]
	mov cr3, rax
	ret



vmm_set_common_kernel_pagemap:
	mov rax, qword [vmm_kernel_pagemap]
	mov qword [vmm_common_kernel_pagemap], rax
	ret



section .common



align 8
vmm_common_kernel_pagemap:
	dq 0

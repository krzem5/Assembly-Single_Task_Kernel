global vmm_switch_to_pagemap
global vmm_set_common_kernel_pagemap
global vmm_common_kernel_pagemap
extern vmm_kernel_pagemap
section .text



[bits 64]
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

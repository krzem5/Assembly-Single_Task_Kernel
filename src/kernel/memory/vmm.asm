global vmm_get_pagemap:function
global vmm_switch_to_pagemap:function
section .text exec nowrite



[bits 64]
vmm_get_pagemap:
	mov rax, cr3
	mov qword [rdi], rax
	ret



vmm_switch_to_pagemap:
	mov rax, qword [rdi]
	mov cr3, rax
	ret

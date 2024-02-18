global vmm_get_pagemap:function hidden
global vmm_switch_to_pagemap:function hidden
section .text exec nowrite



[bits 64]
vmm_get_pagemap:
	mov rax, cr3
	mov qword [rdi], rax
	mov dword [rdi+8], 0
	ret



vmm_switch_to_pagemap:
	mov rax, qword [rdi]
	mov cr3, rax
	ret

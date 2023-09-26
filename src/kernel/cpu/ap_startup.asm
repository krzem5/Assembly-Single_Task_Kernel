%define CPU_AP_STARTUP_MEMORY_ADDRESS 0x2000

%define OFFSET (CPU_AP_STARTUP_MEMORY_ADDRESS-__AP_STARTUP_START__)



extern _cpu_init_core
section .ap_startup exec nowrite



__AP_STARTUP_START__ equ $



[bits 16]
[default rel]
	cli
	cld
	jmp 0x0000:_start16+OFFSET
_start16:
	;;; Initialize data segments
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	;;; Switch to 32-bit mode
	cli
	lgdt [gdt32_descriptor+OFFSET]
	mov eax, cr0
	or eax, 0x1
	mov cr0, eax
	jmp (gdt32_code-gdt32_start):_start32+OFFSET



[bits 32]
[default rel]
_start32:
	;;; Disable interrupts
	cli
	;;; Initialize data segments
	mov ax, (gdt32_data-gdt32_start)
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	;;; Setup paging
	mov eax, dword [kernel_toplevel_pagemap+OFFSET]
	mov cr3, eax
	mov eax, cr4
	or eax, 0x20
	mov cr4, eax
	;;; Enter long mode
	mov ecx, 0xc0000080
	rdmsr
	or eax, 0x900
	wrmsr
	mov eax, cr0
	or eax, 0x80000001
	mov cr0, eax
	lgdt [gdt64_pointer+OFFSET]
	jmp (gdt64_code-gdt64_start):_start64+OFFSET



[bits 64]
[default abs]
_start64:
	;;; Disable interrupts
	cli
	;;; Initialize data segments
	mov ax, (gdt64_data-gdt64_start)
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	;;; Setup stack
	mov rsp, qword [cpu_stack_top+OFFSET]
	xor rbp, rbp
	;;; Start the kernel
	lea rax, _cpu_init_core
	jmp rax



align 4
kernel_toplevel_pagemap:
	dd 0
align 8
cpu_stack_top:
	dq 0
align 16
gdt32_start:
	dq 0x0000000000000000
gdt32_code:
	dq 0x00cf9a000000ffff
gdt32_data:
	dq 0x00cf92000000ffff
gdt32_descriptor:
	dw $-gdt32_start-1
	dd gdt32_start+OFFSET
gdt64_start:
	dq 0x000100000000ffff
gdt64_code:
	dq 0x00af9a0000000000
gdt64_data:
	dq 0x0000920000000000
gdt64_pointer:
	dw $-gdt64_start-1
	dq gdt64_start+OFFSET



times 4096-($-$$) db 0



global cpu_ap_startup_init
global cpu_ap_startup_set_stack_top
extern vmm_kernel_pagemap
extern vmm_map_page
extern vmm_unmap_page
section .text exec nowrite



[bits 64]
cpu_ap_startup_init:
	mov rdi, CPU_AP_STARTUP_MEMORY_ADDRESS
	mov rsi, __AP_STARTUP_START__
	mov rcx, 512
	rep movsq
	mov rax, qword [vmm_kernel_pagemap]
	mov dword [kernel_toplevel_pagemap+OFFSET], eax
	ret



cpu_ap_startup_set_stack_top:
	mov qword [cpu_stack_top+OFFSET], rdi
	ret

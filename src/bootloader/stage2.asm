;;; 00000500 - 00000900: Stage2 bootloader
;;; 00004000 - 00007000: Stack
;;; 00007000 - 00007200: Kernel data
;;; 00100000 - ????????: Kernel code



%define KFS_HEADER_SIZE 4096
%define STACK_BOTTOM 0x4000
%define STACK_TOP 0x7000
%define KERNEL_DATA 0x7000
%define KERNEL_DATA_MMAP_LENGTH word [KERNEL_DATA]
%define KERNEL_DATA_MMAP_PTR (KERNEL_DATA+8)
%define KERNEL_DATA_MMAP_MAX_LEN 42
%define KERNEL_OFFSET 0xffffffffc0000000
%define KERNEL_MEM_ADDR 0x100000



[org 0x500]
section .text



__BOOTLOADER_STAGE2_START__ equ $



[bits 16]
	jmp 0x0000:_start16
_start16:
	;;; Disable interrupts
	cli
	;;; Initialize data segments
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	;;; Store boot drive
	mov byte [boot_drive], dl
	;;; Init stack
	mov bp, STACK_TOP
	mov sp, bp
	;;; Init serial
	mov al, 0x00
	mov dx, 0x3f9
	out dx, al
	mov al, 0x80
	mov dx, 0x3fb
	out dx, al
	mov al, 0x03
	mov dx, 0x3f8
	out dx, al
	mov al, 0x00
	mov dx, 0x3f9
	out dx, al
	mov al, 0x03
	mov dx, 0x3fb
	out dx, al
	mov al, 0xc7
	mov dx, 0x3fa
	out dx, al
	mov al, 0x03
	mov dx, 0x3fc
	out dx, al
	;;; Start bootloader
	mov bx, STRING_BOOTLOADER_STAGE2_16BIT_START
	call ._print
	;;; Read memory map
	mov bx, STRING_READ_MMAP
	call ._print
	xor ebx, ebx
	xor bp, bp
	mov di, KERNEL_DATA_MMAP_PTR
._mmap_read_next_chunk:
	mov eax, 0xe820
	mov ecx, 24
	mov edx, 0x0534d4150
	int 0x15
	cmp eax, 0x0534d4150
	jne ._mmap_end
	or ebx, ebx
	jz ._mmap_end
	cmp ecx, 20
	jle ._mmap_skip_acpi_check
	cmp dword [di+20], 0
	jne ._mmap_read_next_chunk
._mmap_skip_acpi_check:
	add bp, 1
	cmp bp, KERNEL_DATA_MMAP_MAX_LEN
	je ._mmap_end
	add di, 0x18
	jmp ._mmap_read_next_chunk
._mmap_end:
	mov KERNEL_DATA_MMAP_LENGTH, bp
	;;; Load kernel
	mov bx, STRING_LOAD_KERNEL
	call ._print
	mov eax, (0x0200+(__KERNEL_CORE_SIZE__+511)/512)
	mov ebx, 0x7c00
	mov ecx, (0x0002+(__BOOTLOADER_STAGE2_END__-__BOOTLOADER_STAGE2_START__+511)/512+(KFS_HEADER_SIZE+511)/512)
	movzx edx, byte [boot_drive]
	int 0x13
	;;; Switch to 32-bit mode
	mov bx, STRING_SWITCH_TO_32BIT_MODE
	call ._print
	lgdt [gdt32_descriptor]
	mov eax, cr0
	or eax, 0x1
	mov cr0, eax
	jmp (gdt32_code-gdt32_start):_start32
	;;; Serial print function
._print:
	pusha
	mov dx, 0x3f8
._print_next_char:
	add dx, 5
._print_poll_serial:
	in al, dx
	and al, 0x20
	jz ._print_poll_serial
	sub dx, 5
	mov al, byte [bx]
	cmp al, 0
	je ._print_end
	add bx, 1
	out dx, al
	jmp ._print_next_char
._print_end:
	popa
	ret



[bits 32]
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
	;;; Init stack
	mov ebp, STACK_TOP
	mov esp, ebp
	;;; Start 32-bit bootloader
	mov ebx, STRING_BOOTLOADER_STAGE2_32BIT_START
	call ._print32
	;;; Reallocate the kernel
	mov ebx, STRING_REALLOC_KERNEL
	call ._print32
	mov esi, 0x7c00
	mov edi, KERNEL_MEM_ADDR
	mov ecx, ((__KERNEL_CORE_SIZE__+3)/4)
	rep movsd
	;;; Setup paging
	mov ebx, STRING_SETUP_PAGING
	call ._print32
	mov edi, 0x1000
	mov cr3, edi
	xor eax, eax
	mov ecx, 0xc00
	rep stosd
	;;; Map kernel to higher half
	mov ebx, STRING_MAP_KERNEL
	call ._print32
	mov dword [0x1000], 0x00002003
	mov dword [0x1ff8], 0x00003003
	mov dword [0x2000], 0x00000083
	mov dword [0x3ff8], 0x00000083
	mov eax, cr4
	or eax, 0x20
	mov cr4, eax
	;;; Enter long mode
	mov ebx, STRING_SETUP_64BIT_MODE_AND_START_KERNEL
	call ._print32
	mov ecx, 0xc0000080
	rdmsr
	or eax, 0x900
	wrmsr
	mov eax, cr0
	or eax, 0x80000001
	mov cr0, eax
	lgdt [gdt64_pointer]
	jmp (gdt64_code-gdt64_start):_start64
	;;; Serial print function
._print32:
	pusha
	mov dx, 0x3f8
._next_char:
	add dx, 5
._retry:
	in al, dx
	and al, 0x20
	jz ._retry
	sub dx, 5
	mov al, byte [ebx]
	cmp al, 0
	je ._print32_end
	add ebx, 1
	out dx, al
	jmp ._next_char
._print32_end:
	popa
	ret



[bits 64]
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
	;;; Initialize stack
	mov rsp, (KERNEL_OFFSET+STACK_TOP)
	xor rbp, rbp
	;;; Start kernel
	lea rdi, KERNEL_DATA
	jmp (KERNEL_OFFSET+KERNEL_MEM_ADDR)



STRING_BOOTLOADER_STAGE2_16BIT_START: db 10,"Starting stage2 16-bit mode bootloader...",10,0
STRING_READ_MMAP: db "Reading memory map...",10,0
STRING_LOAD_KERNEL: db "Loading kernel...",10,0
STRING_SWITCH_TO_32BIT_MODE: db "Switching to 32-bit mode...",10,0
STRING_BOOTLOADER_STAGE2_32BIT_START: db "Starting stage2 32-bit mode bootloader...",10,0
STRING_REALLOC_KERNEL: db "Reallocating kernel to 1 MB...",10,0
STRING_SETUP_PAGING: db "Setting up paging...",10,0
STRING_MAP_KERNEL: db "Mapping kernel address range 0 - 3fffffff to ffffffffc0000000 - ffffffffffffffff...",10,0
STRING_SETUP_64BIT_MODE_AND_START_KERNEL: db "Setting up 64-bit mode and starting kernel...",10,0



boot_drive: db 0x00
align 16
gdt32_start:
	dq 0x0000000000000000
gdt32_code:
	dq 0x00cf9a000000ffff
gdt32_data:
	dq 0x00cf92000000ffff
gdt32_descriptor:
	dw $-gdt32_start-1
	dd gdt32_start
gdt64_start:
	dq 0x000100000000ffff
gdt64_code:
	dq 0x00af9a0000000000
gdt64_data:
	dq 0x0000920000000000
gdt64_pointer:
	dw $-gdt64_start-1
	dq gdt64_start



times (1024-($-$$)) db 0
__BOOTLOADER_STAGE2_END__ equ $

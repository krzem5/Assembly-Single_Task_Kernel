global compressor_decompress
[bits 64]



compressor_decompress:
	; rax = scratch #1
	; rcx = len
	; rdx = out
	; rdi = data
	; rsi = data_end
	; r8  = src
	; r9  = scratch #2
	add rsi, rdi
	cmp rdi, rsi
	jge ._end
._process_next_header:
	movzx eax, byte [rdi]
	test al, 1
	jz ._read_non_match
	movzx ecx, byte [rdi+1]
	movzx r8d, byte [rdi+2]
	mov r9, rcx
	shr al, 1
	and ecx, 63
	shl r8, 2
	shr r9, 6
	shl ecx, 7
	or ecx, eax
	or r9, r8
	mov r8, rdx
	sub r8, r9
	add rdi, 3
._process_chunk:
	mov rax, rdx
	neg rax
	and eax, 7
	jz ._already_aligned
	mov r9, qword [r8]
	mov qword [rdx], r9
	cmp rax, rcx
	jge ._skip_copy
	sub rcx, rax
	add rdx, rax
	add r8, rax
._already_aligned:
	xor eax, eax
._copy_next_qword:
	mov r9, qword [r8+rax]
	mov qword [rdx+rax], r9
	add rax, 8
	cmp rax, rcx
	jl ._copy_next_qword
._skip_copy:
	add rdx, rcx
	cmp rdi, rsi
	jl ._process_next_header
._end:
	ret
._read_non_match:
	mov ecx, eax
	shr ecx, 2
	test al, 2
	jz ._small_non_match
	movzx eax, byte [rdi+1]
	lea r8, [rdi+2]
	shl eax, 6
	or ecx, eax
	lea rdi, [rdi+rcx+2]
	jmp ._process_chunk
._small_non_match:
	lea r8, [rdi+1]
	lea rdi, [rdi+rcx+1]
	jmp ._process_chunk

extern log
global cpu_check_features
section .ctext



%macro TEST_FEATURE 3
%defstr name %3
	bt %1, %2
	jc ._%3%+_ok
	or r8, 1
	push rcx
	push rdx
	push r8
	lea rdi, _feature_not_present_str
	lea rsi, ._%3%+_str
	call log
	pop r8
	pop rdx
	pop rcx
	jmp ._%3%+_ok
._%3%+_str:
	db name,0
._%3%+_ok:
%endmacro



%macro TEST_FEATURE_WARN 3
%defstr name %3
	bt %1, %2
	jc ._%3%+_ok
	push rcx
	push rdx
	push r8
	lea rdi, _feature_not_present_warn_str
	lea rsi, ._%3%+_str
	call log
	pop r8
	pop rdx
	pop rcx
	jmp ._%3%+_ok
._%3%+_str:
	db name,0
._%3%+_ok:
%endmacro



[bits 64]
cpu_check_features:
	push rbx
	xor r8, r8
	mov eax, 0x00000001
	cpuid
	TEST_FEATURE ecx, 0, sse3
	TEST_FEATURE ecx, 1, pclmulqdq
	TEST_FEATURE ecx, 9, ssse3
	TEST_FEATURE_WARN ecx, 12, fma
	TEST_FEATURE ecx, 19, sse4_1
	TEST_FEATURE ecx, 20, sse4_2
	TEST_FEATURE ecx, 23, popcnt
	; mov r9, rcx
	; shr r9, 23
	; and r9, 2
	; or r9, 1
	; shl r9, 17
	; and r9, rcx
	TEST_FEATURE ecx, 24, tsc_deadline
	TEST_FEATURE ecx, 26, xsave
	TEST_FEATURE_WARN ecx, 28, avx
	TEST_FEATURE_WARN ecx, 29, f16c
	TEST_FEATURE edx, 0, fpu
	TEST_FEATURE edx, 3, pse
	TEST_FEATURE edx, 4, tsc
	TEST_FEATURE edx, 5, msr
	TEST_FEATURE edx, 6, pae
	TEST_FEATURE edx, 9, apic
	TEST_FEATURE edx, 15, cmov
	TEST_FEATURE edx, 23, mmx
	TEST_FEATURE edx, 24, fxsr
	TEST_FEATURE edx, 25, sse
	TEST_FEATURE edx, 26, sse2
	mov eax, 0x00000007
	xor ecx, ecx
	cpuid
	TEST_FEATURE ebx, 0, fsgsbase
	TEST_FEATURE ebx, 3, bmi1
	TEST_FEATURE_WARN ebx, 5, avx2
	TEST_FEATURE ebx, 8, bmi2
	mov eax, 0x80000001
	cpuid
	TEST_FEATURE edx, 11, syscall
	TEST_FEATURE edx, 20, nx
	TEST_FEATURE edx, 26, pdpe1gb
	TEST_FEATURE edx, 27, rdtscp
	mov eax, 0x80000007
	cpuid
	TEST_FEATURE_WARN edx, 8, invtsc
	pop rbx
	test r8, r8
	jnz $
	ret



section .cdata



_feature_not_present_str:
	db 27,"[38;2;65;65;65m[cpu] ",27,"[1m",27,"[38;2;231;72;86mFeature not present: %s",27,"[0m",10,0
_feature_not_present_warn_str:
	db 27,"[38;2;65;65;65m[cpu] ",27,"[38;2;231;211;72mFeature not present: %s",27,"[0m",10,0

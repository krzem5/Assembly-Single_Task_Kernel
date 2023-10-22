#ifndef _KERNEL_TYPES_H_
#define _KERNEL_TYPES_H_ 1



#define KERNEL_BSS __attribute__((section(".bss")))
#define KERNEL_NORETURN __attribute__((noreturn))
#define KERNEL_NOOPT __attribute__((noipa,optimize("O0")))
#define KERNEL_NOCOVERAGE __attribute__((no_instrument_function,no_profile_instrument_function))
#define KERNEL_NOINLINE __attribute__((noinline))
#define KERNEL_INLINE inline __attribute__((always_inline))
#define KERNEL_ATOMIC _Atomic

typedef void (*__KERNEL_TEMP_INIT_FUNC)(void);
#define __KERNEL_TEMP_INIT(init_code) static void _TEMP_INIT_CODE(void){init_code;};static const __KERNEL_TEMP_INIT_FUNC __attribute__((used,section(".tmp_init"))) _TEMP_INIT=(void*)_TEMP_INIT_CODE;
#define __KERNEL_TEMP_INIT_EXECUTE() \
	extern u64 __KERNEL_SECTION_tmp_init_START__[1]; \
	extern u64 __KERNEL_SECTION_tmp_init_END__[1]; \
	for (const __KERNEL_TEMP_INIT_FUNC* AAA=(void*)(u64)__KERNEL_SECTION_tmp_init_START__;(u64)AAA<(u64)__KERNEL_SECTION_tmp_init_END__;AAA++){ \
		(*AAA)(); \
	}



typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;



#endif

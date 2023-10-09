#ifndef _KERNEL_TYPES_H_
#define _KERNEL_TYPES_H_ 1



#define KERNEL_BSS __attribute__((section(".bss")))
#define KERNEL_NORETURN __attribute__((noreturn))
#define KERNEL_NOOPT __attribute__((noipa,optimize("O0")))
#define KERNEL_NOCOVERAGE __attribute__((no_instrument_function,no_profile_instrument_function))
#define KERNEL_INLINE inline __attribute__((always_inline))
#define KERNEL_ATOMIC _Atomic



typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;



#endif

#ifndef _KERNEL_TYPES_H_
#define _KERNEL_TYPES_H_ 1



#define KERNEL_INIT_WRITE __attribute__((section(".idata")))
#define KERNEL_NORETURN __attribute__((noreturn))
#define KERNEL_NOOPT __attribute__((noipa,optimize("O0")))
#define KERNEL_NOCOVERAGE __attribute__((no_instrument_function,no_profile_instrument_function))
#define KERNEL_NOINLINE __attribute__((noinline))
#define KERNEL_INLINE inline __attribute__((always_inline))
#define KERNEL_PACKED __attribute__((packed))
#define KERNEL_ATOMIC _Atomic

#if KERNEL_DISABLE_ASSERT
#define KERNEL_ASSERT(expression,error,...)
#define KERNEL_ASSERT_BLOCK(block)
#else
#define KERNEL_ASSERT(expression,error,...) \
	do{ \
		if (!(expression)){ \
			ERROR(error,##__VA_ARGS__); \
		} \
	} while (0)
#define KERNEL_ASSERT_BLOCK(block) do{block} while(0)
#endif



typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;



#endif

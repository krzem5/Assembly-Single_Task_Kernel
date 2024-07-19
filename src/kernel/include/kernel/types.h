#ifndef _KERNEL_TYPES_H_
#define _KERNEL_TYPES_H_ 1
#include <common/types.h>



#define KERNEL_EARLY_EXEC __attribute__((section(".etext")))
#define KERNEL_EARLY_READ __attribute__((section(".erdata")))
#define KERNEL_EARLY_WRITE __attribute__((section(".edata")))
#define KERNEL_INIT_WRITE __attribute__((section(".idata")))
#define KERNEL_NORETURN __attribute__((noreturn))
#define KERNEL_NOCOVERAGE __attribute__((no_instrument_function,no_profile_instrument_function))
#define KERNEL_NOINLINE __attribute__((noinline))
#ifdef KERNEL_RELEASE
#define KERNEL_INLINE inline __attribute__((always_inline))
#define KERNEL_AWAITS
#define KERNEL_AWAITS_EARLY KERNEL_EARLY_EXEC
#define KERNEL_NO_AWAITS
#else
#define KERNEL_INLINE inline
#define KERNEL_AWAITS __attribute__((constructor,section(".text.await")))
#define KERNEL_AWAITS_EARLY __attribute__((constructor,section(".etext.await")))
#define KERNEL_NO_AWAITS __attribute__((destructor,section(".text.await")))
#endif
#define KERNEL_PUBLIC __attribute__((visibility("default")))
#define KERNEL_PACKED __attribute__((packed))
#define KERNEL_ATOMIC _Atomic
#define KERNEL_USER_POINTER volatile

#define _KERNEL_INITIALIZER_NAME__(a,b) a##b
#define _KERNEL_INITIALIZER_NAME_(a,b) _KERNEL_INITIALIZER_NAME__(a,b)
#define _KERNEL_INITIALIZER_NAME(type) _KERNEL_INITIALIZER_NAME_(type,__UNIQUE_FILE_NAME__)
#define KERNEL_ASYNC_INIT() static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__ainitializer_)(void);static void* __attribute__((section(".ainitializer"),used)) __ainitializer_ptr=_KERNEL_INITIALIZER_NAME(__ainitializer_);static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__ainitializer_)(void)
#define KERNEL_INIT() static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__initializer_)(void);static void* __attribute__((section(".initializer"),used)) __initializer_ptr=_KERNEL_INITIALIZER_NAME(__initializer_);static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__initializer_)(void)
#define KERNEL_EARLY_INIT() static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__einitializer_)(void);static void* __attribute__((section(".einitializer"),used)) __einitializer_ptr=_KERNEL_INITIALIZER_NAME(__einitializer_);static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__einitializer_)(void)
#define KERNEL_EARLY_EARLY_INIT() static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__eeinitializer_)(void);static void* __attribute__((section(".eeinitializer"),used)) __eeinitializer_ptr=_KERNEL_INITIALIZER_NAME(__eeinitializer_);static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__eeinitializer_)(void)
#define KERNEL_EARLY_POINTER(name) static void* __attribute__((section(".epointer"),used)) __epointer_##name=&(name)

#ifdef KERNEL_RELEASE
#define KERNEL_ASSERT(expression)
#else
#define KERNEL_ASSERT(expression) \
	do{ \
		if (!(expression)){ \
			ERROR("[line %u] "#expression": Assertion failed",__LINE__); \
		} \
	} while (0)
#endif



#define KERNEL_OFFSETOF(type,field) __builtin_offsetof(type,field)
#define KERNEL_CONTAINEROF(object,type,field) ((type*)(((u64)(void*)(object))-KERNEL_OFFSETOF(type,field)))



#endif

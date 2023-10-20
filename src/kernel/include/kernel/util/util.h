#ifndef _KERNEL_UTIL_UTIL_H_
#define _KERNEL_UTIL_UTIL_H_ 1
#include <kernel/types.h>



#define memcpy(dst,src,length) ((__builtin_constant_p(length)?__memcpy_inline:(memcpy))((dst),(src),(length)))
#define memset(dst,src,length) ((__builtin_constant_p(length)?__memset_inline:(memset))((dst),(src),(length)))



#if KERNEL_COVERAGE_ENABLED
#define SPINLOOP(cond) \
	do{ \
		KERNEL_INLINE void KERNEL_NOCOVERAGE __nocoverage_spinloop(void){ \
			while (cond){ \
				__pause(); \
			} \
		} \
		__nocoverage_spinloop(); \
	} while(0)
#define COUNTER_SPINLOOP(max) \
	do{ \
		KERNEL_INLINE void KERNEL_NOCOVERAGE __nocoverage_counter_spinloop(void){ \
			for (u64 __tmp=0;__tmp<(max);__tmp++){ \
				__pause(); \
			} \
		} \
		__nocoverage_counter_spinloop(); \
	} while(0)
#else
#define SPINLOOP(cond) \
	while (cond){ \
		__pause(); \
	}
#define COUNTER_SPINLOOP(max) \
	for (u64 __tmp=0;__tmp<(max);__tmp++){ \
		__pause(); \
	}
#endif



static KERNEL_INLINE void KERNEL_NOCOVERAGE __pause(void){
	asm volatile("pause":::"memory");
}



static KERNEL_INLINE void* KERNEL_NOCOVERAGE __memcpy_inline(void* dst,const void* src,u64 length){
	u8* dst_ptr=dst;
	const u8* src_ptr=src;
	for (u64 i=0;i<length;i++){
		dst_ptr[i]=src_ptr[i];
	}
	return dst;
}



static KERNEL_INLINE void* KERNEL_NOCOVERAGE __memset_inline(void* dst,u8 value,u64 length){
	u8* ptr=dst;
	for (u64 i=0;i<length;i++){
		ptr[i]=value;
	}
	return dst;
}



void* (memcpy)(void* dst,const void* src,u64 length);



void* (memset)(void* dst,u8 value,u64 length);



_Bool streq(const char* a,const char* b);



char* memcpy_lowercase(char* dst,const char* src,u64 length);



void memcpy_trunc_spaces(char* dst,const char* src,u8 length);



void bswap16_trunc_spaces(const u16* src,u8 length,char* dst);



void KERNEL_NORETURN panic(const char* error);



#endif

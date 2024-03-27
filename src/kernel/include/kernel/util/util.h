#ifndef _KERNEL_UTIL_UTIL_H_
#define _KERNEL_UTIL_UTIL_H_ 1
#include <kernel/types.h>



#define memcpy(dst,src,length) ((__builtin_constant_p(length)?__memcpy_inline:(memcpy))((dst),(src),(length)))
#define memset(dst,src,length) ((__builtin_constant_p(length)?__memset_inline:(memset))((dst),(src),(length)))



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



_Bool str_equal(const char* a,const char* b);



void str_copy(char* dst,const char* src,u64 max_length);



void str_copy_from_padded(const char* src,char* dst,u64 length);



void str_copy_byte_swap_from_padded(const u16* src,char* dst,u64 length);



void KERNEL_NORETURN panic(const char* error);



#endif

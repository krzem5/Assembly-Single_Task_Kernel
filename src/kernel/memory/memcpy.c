#include <kernel/types.h>



void* KERNEL_CORE_CODE KERNEL_NOOPT memcpy(void* dst,const void* src,u64 length){
	const u8* src_ptr=src;
	u8* dst_ptr=dst;
	for (;length;length--){
		*dst_ptr=*src_ptr;
		src_ptr++;
		dst_ptr++;
	}
	return dst;
}

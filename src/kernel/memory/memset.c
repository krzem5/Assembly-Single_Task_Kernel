#include <kernel/types.h>



void* KERNEL_CORE_CODE KERNEL_NOOPT memset(void* dst,u8 value,u64 length){
	for (u8* ptr=dst;length;length--){
		*ptr=value;
		ptr++;
	}
	return dst;
}

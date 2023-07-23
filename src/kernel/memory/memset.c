#include <kernel/types.h>



void* __attribute__((noipa,optimize("O0"))) memset(void* dst,u8 value,u64 length){
	for (u8* ptr=dst;length;length--){
		*ptr=value;
		ptr++;
	}
	return dst;
}

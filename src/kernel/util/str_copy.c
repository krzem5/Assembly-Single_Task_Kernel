#include <kernel/types.h>
#include <kernel/util/util.h>



KERNEL_PUBLIC void KERNEL_NOCOVERAGE str_copy(char* dst,const char* src,u64 max_length){
	if (!max_length){
		return;
	}
	for (u64 i=0;i<max_length-1;i++){
		dst[i]=src[i];
		if (!src[i]){
			break;
		}
	}
	dst[max_length-1]=0;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE str_copy_from_padded(const char* src,char* dst,u64 length){
	for (;length&&src[length-1]==32;length--);
	memcpy(dst,src,length);
	dst[length]=0;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE str_copy_byte_swap_from_padded(const u16* src,char* dst,u64 length){
	u16* dst16=(u16*)dst;
	for (u64 i=0;i<length;i++){
		dst16[i]=__builtin_bswap16(src[i]);
	}
	u64 i=length<<1;
	do{
		i--;
	} while (i&&dst[i-1]==32);
	dst[i]=0;
}

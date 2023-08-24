#include <kernel/types.h>



void KERNEL_CORE_CODE drive_change_byte_order_and_truncate_spaces(const u16* src,u8 length,char* dst){
	u16* dst16=(u16*)dst;
	for (u8 i=0;i<length;i++){
		dst16[i]=__builtin_bswap16(src[i]);
	}
	u8 i=length<<1;
	do{
		i--;
	} while (i&&dst[i-1]==32);
	dst[i]=0;
}

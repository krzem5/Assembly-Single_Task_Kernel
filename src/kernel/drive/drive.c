#include <kernel/types.h>



void drive_change_byte_order_and_truncate_spaces(const u16* src,u8 length,char* dst){
	u8 i=0;
	for (;i<length;i++){
		dst[i<<1]=src[i]>>8;
		dst[(i<<1)+1]=src[i];
	}
	dst[i]=32;
	while (i&&dst[i-1]==32){
		i--;
	}
	dst[i]=0;
}

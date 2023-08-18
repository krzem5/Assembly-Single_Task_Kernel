#include <kernel/types.h>



void KERNEL_CORE_CODE drive_insert_index_into_name(const char* name,u8 index,char* dst){
	while (*name){
		*dst=*name;
		name++;
		dst++;
	}
	if (index>99){
		dst[0]=index/100+48;
		dst[1]=((index/10)%10)+48;
		dst[2]=(index%10)+48;
		dst[3]=0;
	}
	else if (index>9){
		dst[0]=index/10+48;
		dst[1]=(index%10)+48;
		dst[2]=0;
	}
	else{
		dst[0]=index+48;
		dst[1]=0;
	}
}



void KERNEL_CORE_CODE drive_change_byte_order_and_truncate_spaces(const u16* src,u8 length,char* dst){
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

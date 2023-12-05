#include <kernel/types.h>



static void _update_checksum(const void* data,u16 length,u32 checksum,u16* out){
	for (u16 i=0;i<length;i+=2){
		checksum+=*((const u16*)(data+i));
	}
	checksum=(checksum&0xffff)+(checksum>>16);
	*out=~(checksum+(checksum>>16));
}



KERNEL_PUBLIC void net_checksum_calculate_checksum(const void* data,u16 length,u16* out){
	*out=0;
	_update_checksum(data,length,0,out);
}



KERNEL_PUBLIC void net_checksum_update_checksum(const void* data,u16 length,u16* out){
	_update_checksum(data,length,(~(*out))&0xffff,out);
}



KERNEL_PUBLIC u16 net_checksum_verify_checksum(const void* data,u16 length,u32 checksum){
	for (u16 i=0;i<length;i+=2){
		checksum+=*((const u16*)(data+i));
	}
	checksum=(checksum&0xffff)+(checksum>>16);
	return checksum+(checksum>>16);
}

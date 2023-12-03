#include <kernel/module/module.h>
#include <kernel/types.h>
#include <net/common.h>
#include <net/ip4.h>



static void _update_checksum(const void* data,u16 length,u32 checksum,u16* out){
	for (u16 i=0;i<length;i+=2){
		checksum+=*((const u16*)(data+i));
	}
	checksum=(checksum&0xffff)+(checksum>>16);
	*out=~(checksum+(checksum>>16));
}



KERNEL_PUBLIC net_ip4_address_t net_ip4_address=0x00000000;



KERNEL_PUBLIC void net_common_calculate_checksum(const void* data,u16 length,u16* out){
	*out=0;
	_update_checksum(data,length,0,out);
}



KERNEL_PUBLIC void net_common_update_checksum(const void* data,u16 length,u16* out){
	_update_checksum(data,length,(~(*out))&0xffff,out);
}



KERNEL_PUBLIC u16 net_common_verify_checksum(const void* data,u16 length,u32 checksum){
	for (u16 i=0;i<length;i+=2){
		checksum+=*((const u16*)(data+i));
	}
	checksum=(checksum&0xffff)+(checksum>>16);
	return checksum+(checksum>>16);
}



static _Bool _init(module_t* module){
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
